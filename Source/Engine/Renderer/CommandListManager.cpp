#include "Pch.h"
#include "Renderer/CommandListManager.h"

namespace esperanza
{
	// Lifetime of these objects is managed by the descriptor cache
	CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type) noexcept
		: m_pCommandQueue()
		, m_Type(type)
		, m_AllocatorPool(type)
		, m_FenceMutex()
		, m_EventMutex()
		, m_pFence()
		, m_uNextFenceValue(static_cast<UINT64>(type) << 56 | 1)
		, m_uLastCompletedFenceValue(static_cast<UINT64>(type) << 56)
		, m_hFenceEventHandle()
	{
	}

	HRESULT CommandQueue::Initialize(ID3D12Device* pDevice) noexcept
	{
		HRESULT hr = S_OK;

		if (!pDevice)
		{
			GLOGE(L"Device is null!");

			return E_FAIL;
		}

		if (IsReady())
		{
			GLOGE(L"Command queue is already initialized!");

			return E_FAIL;
		}

		if (m_AllocatorPool.GetSize())
		{
			GLOGE(L"Command allocator pool is not empty!");

			return E_FAIL;
		}

		D3D12_COMMAND_QUEUE_DESC queueDesc =
		{
			.Type = m_Type,
			.NodeMask = 1,
		};

		hr = pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Creating Command Queue failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}
		m_pCommandQueue->SetName(L"CommandListManager::m_pCommandQueue");

		hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Creating Fence failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}
		m_pFence->SetName(L"CommandListManager::m_pFence");
		m_pFence->Signal(static_cast<UINT64>(m_Type) << 56);

		m_hFenceEventHandle = CreateEvent(NULL, FALSE, FALSE, nullptr);
		if (!m_hFenceEventHandle)
		{
			DWORD dwError = GetLastError();

			if (dwError != ERROR_CLASS_ALREADY_EXISTS)
			{
				_com_error err(hr);
				GLOGEF(L"Creating Fence Event Handle failed with HRESULT code %u, %s", hr, err.ErrorMessage());
				return HRESULT_FROM_WIN32(dwError);
			}

			GLOGEF(L"Creating Fence Event Handle failed with DWORD code %u", dwError);

			return E_FAIL;
		}

		m_AllocatorPool.Initialize(pDevice);

		if (!IsReady())
		{
			GLOGE(L"Command Queue Initialization failed. Command Queue is not ready");
			return E_FAIL;
		}

		return hr;
	}

	void CommandQueue::Destroy() noexcept
	{
		if (!m_pCommandQueue)
		{
			return;
		}

		m_AllocatorPool.Destroy();

		CloseHandle(m_hFenceEventHandle);
	}

	HRESULT CommandQueue::IncrementFence(_Out_ UINT64& uNextFenceValue) noexcept
	{
		std::lock_guard<std::mutex> lockGuard(m_FenceMutex);

		HRESULT hr = S_OK;

		hr = m_pCommandQueue->Signal(m_pFence.Get(), m_uNextFenceValue);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Signaling failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		uNextFenceValue = m_uNextFenceValue++;

		return hr;
	}

	BOOL CommandQueue::IsFenceComplete(UINT64 uFenceValue) noexcept
	{
		// Avoid querying the fence value by testing against the last one seen.
		// The max() is to protect against an unlikely race condition that could cause the last
		// completed fence value to regress.
		if (uFenceValue > m_uLastCompletedFenceValue)
		{
			m_uLastCompletedFenceValue = std::max(m_uLastCompletedFenceValue, m_pFence->GetCompletedValue());
		}

		return uFenceValue <= m_uLastCompletedFenceValue;
	}

	HRESULT CommandQueue::StallForFence(CommandListManager& commandListManager, UINT64 uFenceValue) noexcept
	{
		HRESULT hr = S_OK;

		CommandQueue& producer = commandListManager.GetQueue(static_cast<D3D12_COMMAND_LIST_TYPE>(uFenceValue >> 56));

		hr = m_pCommandQueue->Wait(producer.m_pFence.Get(), uFenceValue);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Waiting producer fence value failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		return hr;
	}

	HRESULT CommandQueue::StallForProducer(CommandQueue& producer) noexcept
	{
		HRESULT hr = S_OK;

		if (producer.m_uNextFenceValue <= 0)
		{
			GLOGE(L"Producer's Next Fence Value is zero or negative!");

			return E_FAIL;
		}

		hr = m_pCommandQueue->Wait(producer.m_pFence.Get(), producer.m_uNextFenceValue - 1);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Waiting producer fence value failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		return hr;
	}

	HRESULT CommandQueue::WaitForFence(UINT64 uFenceValue) noexcept
	{
		HRESULT hr = S_OK;

		if (IsFenceComplete(uFenceValue))
		{
			return hr;
		}

		// TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
		// wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
		// the fence can only have one event set on completion, then thread B has to wait for 
		// 100 before it knows 99 is ready.  Maybe insert sequential events?
		{
			std::lock_guard<std::mutex> lockGuard(m_EventMutex);

			hr = m_pFence->SetEventOnCompletion(uFenceValue, m_hFenceEventHandle);
			if (FAILED(hr))
			{
				_com_error err(hr);
				GLOGEF(L"Set fence event on completion failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}

			WaitForSingleObject(m_hFenceEventHandle, INFINITE);
			m_uLastCompletedFenceValue = uFenceValue;
		}

		return hr;
	}

	HRESULT CommandQueue::WaitForIdle() noexcept
	{
		HRESULT hr = S_OK;
		UINT64 uFenceValue = 0;
		hr = IncrementFence(uFenceValue);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Incrementing Fence failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		hr = WaitForFence(uFenceValue);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Wait For Fence failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}
		return hr;
	}

	BOOL CommandQueue::IsReady() const noexcept
	{
		return !!m_pCommandQueue;
	}

	ID3D12CommandQueue* CommandQueue::GetCommandQueue() noexcept
	{
		return m_pCommandQueue.Get();
	}

	const ID3D12CommandQueue* CommandQueue::GetCommandQueue() const noexcept
	{
		return m_pCommandQueue.Get();
	}

	constexpr UINT64 CommandQueue::GetNextFenceValue() const noexcept
	{
		return m_uNextFenceValue;
	}

	HRESULT CommandQueue::executeCommandList(UINT64& uOutNextFenceValue, ID3D12CommandList* pList) noexcept
	{
		std::lock_guard<std::mutex> lockGuard(m_FenceMutex);

		HRESULT hr = S_OK;

		hr = static_cast<ID3D12GraphicsCommandList*>(pList)->Close();
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Closing Command List failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		// Kickoff the command list
		m_pCommandQueue->ExecuteCommandLists(1, &pList);

		// Signal the next fence value (with the GPU)
		hr = m_pCommandQueue->Signal(m_pFence.Get(), m_uNextFenceValue);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Signaling failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		uOutNextFenceValue = m_uNextFenceValue++;

		return hr;
	}

	HRESULT CommandQueue::requestAllocator(_Out_ ID3D12CommandAllocator** ppOutAllocator) noexcept
	{
		HRESULT hr = S_OK;
		UINT64 uCompletedFence = m_pFence->GetCompletedValue();

		*ppOutAllocator = nullptr;
		hr = m_AllocatorPool.RequestAllocator(ppOutAllocator, uCompletedFence);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Requesting allocator from pool failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		return hr;
	}

	void CommandQueue::discardAllocator(UINT64 uFenceValueForReset, ID3D12CommandAllocator* pAllocator)
	{
		m_AllocatorPool.DiscardAllocator(uFenceValueForReset, pAllocator);
	}

	CommandListManager::CommandListManager() noexcept
		: m_pDevice()
		, m_GraphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)
		, m_ComputeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)
		, m_CopyQueue(D3D12_COMMAND_LIST_TYPE_COPY)
	{
	}

	HRESULT CommandListManager::Initialize(ID3D12Device* pDevice) noexcept
	{
		HRESULT hr = S_OK;

		if (!pDevice)
		{
			GLOGE(L"Device is null!");

			return E_FAIL;
		}

		m_pDevice = pDevice;

		m_GraphicsQueue.Initialize(pDevice);
		m_ComputeQueue.Initialize(pDevice);
		m_CopyQueue.Initialize(pDevice);

		return hr;
	}

	void CommandListManager::Destroy() noexcept
	{
		m_GraphicsQueue.Destroy();
		m_ComputeQueue.Destroy();
		m_CopyQueue.Destroy();
	}

	constexpr CommandQueue& CommandListManager::GetGraphicsQueue() noexcept
	{
		return m_GraphicsQueue;
	}

	constexpr const CommandQueue& CommandListManager::GetGraphicsQueue() const noexcept
	{
		return m_GraphicsQueue;
	}

	constexpr CommandQueue& CommandListManager::GetComputeQueue() noexcept
	{
		return m_ComputeQueue;
	}

	constexpr const CommandQueue& CommandListManager::GetComputeQueue() const noexcept
	{
		return m_ComputeQueue;
	}

	constexpr CommandQueue& CommandListManager::GetCopyQueue() noexcept
	{
		return m_CopyQueue;
	}

	constexpr const CommandQueue& CommandListManager::GetCopyQueue() const noexcept
	{
		return m_CopyQueue;
	}

	constexpr CommandQueue& CommandListManager::GetQueue(D3D12_COMMAND_LIST_TYPE type) noexcept
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return m_GraphicsQueue;
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return m_ComputeQueue;
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return m_CopyQueue;
			break;
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
			[[fallthrough]];
		default:
			assert(false);
			break;
		}

		return m_GraphicsQueue;
	}

	constexpr const CommandQueue& CommandListManager::GetQueue(D3D12_COMMAND_LIST_TYPE type) const noexcept
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return m_GraphicsQueue;
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return m_ComputeQueue;
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return m_CopyQueue;
			break;
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
			[[fallthrough]];
		default:
			assert(false);
			break;
		}
	}

	constexpr CommandQueue& CommandListManager::GetQueue() noexcept
	{
		return GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	constexpr const CommandQueue& CommandListManager::GetQueue() const noexcept
	{
		return GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	ID3D12CommandQueue* CommandListManager::GetCommandQueue() noexcept
	{
		return m_GraphicsQueue.GetCommandQueue();
	}

	const ID3D12CommandQueue* CommandListManager::GetCommandQueue() const noexcept
	{
		return m_GraphicsQueue.GetCommandQueue();
	}

	HRESULT CommandListManager::CreateNewCommandList(ID3D12GraphicsCommandList** ppOutList, ID3D12CommandAllocator** ppOutAllocator, D3D12_COMMAND_LIST_TYPE type) noexcept
	{
		HRESULT hr = S_OK;
		
		if (type == D3D12_COMMAND_LIST_TYPE_BUNDLE)
		{
			GLOGE(L"Bundles are not yet supported!");

			return E_FAIL;
		}

		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			hr = m_GraphicsQueue.requestAllocator(ppOutAllocator);
			if (FAILED(hr))
			{
				_com_error err(hr);
				GLOGEF(L"Requesting allocator failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			hr = m_ComputeQueue.requestAllocator(ppOutAllocator);
			if (FAILED(hr))
			{
				_com_error err(hr);
				GLOGEF(L"Requesting allocator failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			hr = m_CopyQueue.requestAllocator(ppOutAllocator);
			if (FAILED(hr))
			{
				_com_error err(hr);
				GLOGEF(L"Requesting allocator failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}
			break;
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
			[[fallthrough]];
		case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
			[[fallthrough]];
		default:
		{
			GLOGEF(L"These types(%u) are not yet supported!", static_cast<UINT>(type));

			return E_FAIL;
		}
			break;
		}

		hr = m_pDevice->CreateCommandList(1u, type, *ppOutAllocator, nullptr, IID_PPV_ARGS(ppOutList));
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Creating Command List failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}
		(*ppOutList)->SetName(L"CommandList");

		return hr;
	}

	void CommandListManager::IdleGpu() noexcept
	{
		m_GraphicsQueue.WaitForIdle();
		m_ComputeQueue.WaitForIdle();
		m_CopyQueue.WaitForIdle();
	}
}