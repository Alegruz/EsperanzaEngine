#include "Pch.h"

#include "Renderer/DescriptorHeap.h"

namespace esperanza
{
	std::mutex DescriptorAllocator::sm_AllocationMutex;
	std::vector<ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::sm_DescriptorHeapPool;

	void DescriptorAllocator::DestroyAll(void)
	{
		sm_DescriptorHeapPool.clear();
	}

	DescriptorAllocator::DescriptorAllocator(ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type) noexcept
		: m_pDevice(pDevice)
		, m_Type(type)
		, m_pCurrentHeap(nullptr)
		, m_CurrentHandle()
		, m_uDescriptorSize(0)
		, m_uRemainingFreeHandles()
	{
		m_CurrentHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	HRESULT DescriptorAllocator::Allocate(D3D12_CPU_DESCRIPTOR_HANDLE& outHandle, ID3D12Device* pDevice, uint32_t uCount)
	{
		HRESULT hr = S_OK;

		if (!m_pCurrentHeap || m_uRemainingFreeHandles < uCount)
		{
			hr = requestNewHeap(&m_pCurrentHeap, pDevice, m_Type);
			if (FAILED(hr))
			{
				_com_error err(hr);
				GLOGEF(L"Requesting New Heap failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}

			m_CurrentHandle = m_pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
			m_uRemainingFreeHandles = NUM_DESCRIPTORS_PER_HEAP;

			if (m_uDescriptorSize == 0)
			{
				m_uDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(m_Type);
			}
		}

		outHandle = m_CurrentHandle;
		m_CurrentHandle.ptr += uCount * m_uDescriptorSize;
		m_uRemainingFreeHandles -= uCount;

		return hr;
	}

	HRESULT DescriptorAllocator::requestNewHeap(ID3D12DescriptorHeap** ppOutDescriptorHeap, ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		std::lock_guard<std::mutex> lockGuard(sm_AllocationMutex);

		HRESULT hr = S_OK;
		*ppOutDescriptorHeap = nullptr;

		D3D12_DESCRIPTOR_HEAP_DESC desc =
		{
			.Type = type,
			.NumDescriptors = NUM_DESCRIPTORS_PER_HEAP,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 1,
		};

		ComPtr<ID3D12DescriptorHeap> pHeap;
		hr = pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap));
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Creating Descriptor Heap failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		sm_DescriptorHeapPool.emplace_back(pHeap);
		*ppOutDescriptorHeap = pHeap.Get();

		return hr;
	}

	
	DescriptorHandle::DescriptorHandle() noexcept
	{
		m_CpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_GpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	DescriptorHandle::DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) noexcept
		: m_CpuHandle(cpuHandle)
		, m_GpuHandle(gpuHandle)
	{
	}

	DescriptorHandle DescriptorHandle::operator+(INT nOffsetScaledByDescriptorSize) const noexcept
	{
		DescriptorHandle ret = *this;
		ret += nOffsetScaledByDescriptorSize;
		return ret;
	}

	void DescriptorHandle::operator+=(INT nOffsetScaledByDescriptorSize) noexcept
	{
		if (m_CpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_CpuHandle.ptr += nOffsetScaledByDescriptorSize;
		}

		if (m_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_GpuHandle.ptr += nOffsetScaledByDescriptorSize;
		}
	}

	HRESULT DescriptorHeap::Initialize(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strDebugHeapName, _In_ D3D12_DESCRIPTOR_HEAP_TYPE type, _In_ uint32_t uMaxCount) noexcept
	{
		HRESULT hr = S_OK;
		m_HeapDesc.Type = type;
		m_HeapDesc.NumDescriptors = uMaxCount;
		m_HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_HeapDesc.NodeMask = 1;

		hr = pDevice->CreateDescriptorHeap(&m_HeapDesc, IID_PPV_ARGS(&m_pHeap));
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Creating Descriptor Heap failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

#ifdef _DEBUG
		m_pHeap->SetName(strDebugHeapName.c_str());
#else
		UNREFERENCED_PARAMETER(strDebugHeapName);
#endif
		
		m_uDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(m_HeapDesc.Type);
		m_uNumFreeDescriptors = m_HeapDesc.NumDescriptors;
		m_FirstHandle = DescriptorHandle(
			m_pHeap->GetCPUDescriptorHandleForHeapStart(),
			m_pHeap->GetGPUDescriptorHandleForHeapStart()
		);
		m_NextFreeHandle = m_FirstHandle;

		return hr;
	}

	void DescriptorHeap::Destroy() noexcept
	{
		m_pHeap.Reset();
	}

	DescriptorHandle DescriptorHeap::Alloc() noexcept
	{
		return Alloc(1);
	}

	DescriptorHandle DescriptorHeap::Alloc(uint32_t uCount) noexcept
	{
		if (!HasAvailableSpace(uCount))
		{
			GLOGAS(L"Descriptor Heap out of space.  Increase heap size.");
			assert(false);
		}

		DescriptorHandle ret = m_NextFreeHandle;
		m_NextFreeHandle += uCount * m_uDescriptorSize;
		m_uNumFreeDescriptors -= uCount;
		return ret;
	}

	DescriptorHandle DescriptorHeap::operator[](uint32_t uArrayIdx) const noexcept
	{
		return m_FirstHandle + uArrayIdx * m_uDescriptorSize;
	}

	BOOL DescriptorHeap::ValidateHandle(const DescriptorHandle& dHandle) const noexcept
	{
		if (dHandle.GetCpuPtr() < m_FirstHandle.GetCpuPtr() ||
			dHandle.GetCpuPtr() >= m_FirstHandle.GetCpuPtr() + m_HeapDesc.NumDescriptors * m_uDescriptorSize)
		{
			return FALSE;
		}

		if (dHandle.GetGpuPtr() - m_FirstHandle.GetGpuPtr() !=
			dHandle.GetCpuPtr() - m_FirstHandle.GetCpuPtr())
		{
			return FALSE;
		}

		return TRUE;
	}

	ID3D12DescriptorHeap* DescriptorHeap::GetHeapPointer() const noexcept
	{
		return m_pHeap.Get();
	}
}