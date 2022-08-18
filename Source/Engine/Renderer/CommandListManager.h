#pragma once

#include "Pch.h"
#include "Renderer/CommandAllocatorPool.h"

namespace esperanza
{
	class CommandQueue final
	{
		friend class CommandListManager;
		friend class CommandContext;

	public:
		CommandQueue() = delete;
		explicit CommandQueue(D3D12_COMMAND_LIST_TYPE type) noexcept;
		CommandQueue(const CommandQueue& other) = delete;
		CommandQueue(CommandQueue&& other) = delete;
		CommandQueue& operator=(const CommandQueue& other) = delete;
		CommandQueue& operator=(CommandQueue&& other) = delete;
		~CommandQueue() noexcept = default;

		HRESULT Initialize(_In_ ID3D12Device* pDevice) noexcept;
		void Destroy() noexcept;

		HRESULT IncrementFence(_Out_ UINT64& uNextFenceValue) noexcept;
		BOOL IsFenceComplete(_In_ UINT64 uFenceValue) noexcept;
		HRESULT StallForFence(_In_ CommandListManager& commandListManager, _In_ UINT64 uFenceValue) noexcept;
		HRESULT StallForProducer(_In_ CommandQueue& producer) noexcept;
		HRESULT WaitForFence(_In_ UINT64 uFenceValue) noexcept;
		HRESULT WaitForIdle() noexcept;

		BOOL IsReady() const noexcept;
		ID3D12CommandQueue* GetCommandQueue() noexcept;
		const ID3D12CommandQueue* GetCommandQueue() const noexcept;
		constexpr UINT64 GetNextFenceValue() const noexcept;

	private:
		HRESULT executeCommandList(_Out_ UINT64& uOutNextFenceValue, _In_ ID3D12CommandList* pList) noexcept;
		HRESULT requestAllocator(_Out_ ID3D12CommandAllocator** ppOutAllocator) noexcept;
		void discardAllocator(_In_ UINT64 uFenceValueForReset, _In_ ID3D12CommandAllocator* pAllocator);

	private:
		ComPtr<ID3D12CommandQueue> m_pCommandQueue;
		const D3D12_COMMAND_LIST_TYPE m_Type;
		CommandAllocatorPool m_AllocatorPool;
		std::mutex m_FenceMutex;
		std::mutex m_EventMutex;

		// Lifetime of these objects is managed by the descriptor cache
		ComPtr<ID3D12Fence> m_pFence;
		UINT64 m_uNextFenceValue;
		UINT64 m_uLastCompletedFenceValue;
		HANDLE m_hFenceEventHandle;
	};

	class CommandListManager final
	{
		friend class CommandContext;
	public:
		explicit CommandListManager() noexcept;
		CommandListManager(const CommandListManager& other) = delete;
		CommandListManager(CommandListManager&& other) = delete;
		CommandListManager& operator=(const CommandListManager& other) = delete;
		CommandListManager& operator=(CommandListManager&& other) = delete;
		~CommandListManager() noexcept = default;

		HRESULT Initialize(_In_ ID3D12Device* pDevice) noexcept;
		void Destroy() noexcept;

		constexpr CommandQueue& GetGraphicsQueue() noexcept;
		constexpr const CommandQueue& GetGraphicsQueue() const noexcept;
		constexpr CommandQueue& GetComputeQueue() noexcept;
		constexpr const CommandQueue& GetComputeQueue() const noexcept;
		constexpr CommandQueue& GetCopyQueue() noexcept;
		constexpr const CommandQueue& GetCopyQueue() const noexcept;

		constexpr CommandQueue& GetQueue(_In_ D3D12_COMMAND_LIST_TYPE type) noexcept;
		constexpr const CommandQueue& GetQueue(_In_ D3D12_COMMAND_LIST_TYPE type) const noexcept;
		constexpr CommandQueue& GetQueue() noexcept;
		constexpr const CommandQueue& GetQueue() const noexcept;

		ID3D12CommandQueue* GetCommandQueue() noexcept;
		const ID3D12CommandQueue* GetCommandQueue() const noexcept;

		HRESULT CreateNewCommandList(_Out_ ID3D12GraphicsCommandList** ppOutList, _Out_ ID3D12CommandAllocator** ppOutAllocator, _In_ D3D12_COMMAND_LIST_TYPE type) noexcept;
		constexpr BOOL IsFenceComplete(_In_ UINT64 uFenceValue) noexcept;

		void WaitForFence(_In_ UINT64 uFenceValue) noexcept;
		void IdleGpu() noexcept;

	private:
		ComPtr<ID3D12Device> m_pDevice;
		CommandQueue m_GraphicsQueue;
		CommandQueue m_ComputeQueue;
		CommandQueue m_CopyQueue;
	};
}