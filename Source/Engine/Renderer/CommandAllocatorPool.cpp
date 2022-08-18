#include "Pch.h"
#include "CommandAllocatorPool.h"

namespace esperanza
{
	CommandAllocatorPool::CommandAllocatorPool(_In_ D3D12_COMMAND_LIST_TYPE type) noexcept
		: m_Type(type)
		, m_pDevice()
		, m_AllocatorPool()
		, m_ReadyAllocators()
		, m_AllocatorMutex()
	{
	}


	void CommandAllocatorPool::Initialize(_In_ ID3D12Device* pDevice) noexcept
	{
		m_pDevice = pDevice;
	}

	void CommandAllocatorPool::Destroy()
	{
		m_AllocatorPool.clear();
	}

	HRESULT CommandAllocatorPool::RequestAllocator(_Out_ ID3D12CommandAllocator** ppAllocator, _In_ UINT64 uCompletedFenceValue) noexcept
	{
		std::lock_guard<std::mutex> lockGuard(m_AllocatorMutex);

		HRESULT hr = S_OK;
		ID3D12CommandAllocator* pAllocator = nullptr;

		if (!m_ReadyAllocators.empty())
		{
			std::pair<UINT64, ID3D12CommandAllocator*>& allocatorPair = m_ReadyAllocators.front();

			if (allocatorPair.first <= uCompletedFenceValue)
			{
				pAllocator = allocatorPair.second;
				hr = pAllocator->Reset();
				if (FAILED(hr))
				{
					_com_error err(hr);
					GLOGEF(L"Resetting Command Allocator failed with HRESULT code %u, %s", hr, err.ErrorMessage());

					return hr;
				}
				m_ReadyAllocators.pop();
			}
		}

		// If no allocators were ready to be reused, create a new one
		if (!pAllocator)
		{
			hr = m_pDevice->CreateCommandAllocator(m_Type, IID_PPV_ARGS(&pAllocator));
			if (FAILED(hr))
			{
				_com_error err(hr);
				GLOGEF(L"Creating Command Allocator failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}
			WCHAR szAllocatorName[32];
			swprintf_s(szAllocatorName, L"CommandAllocator %zu", m_AllocatorPool.size());
			pAllocator->SetName(szAllocatorName);
			m_AllocatorPool.push_back(pAllocator);
		}

		*ppAllocator = pAllocator;

		return hr;
	}

	void CommandAllocatorPool::DiscardAllocator(_In_ UINT64 uFenceValue, _In_ ID3D12CommandAllocator* pAllocator) noexcept
	{
		std::lock_guard<std::mutex> lockGuard(m_AllocatorMutex);

		// That fence value indicates we are free to reset the allocator
		m_ReadyAllocators.push(std::make_pair(uFenceValue, pAllocator));
	}
}