#pragma once

#include "Pch.h"

namespace esperanza
{
	class CommandAllocatorPool final
	{
	public:
		CommandAllocatorPool() = delete;
		explicit CommandAllocatorPool(_In_ D3D12_COMMAND_LIST_TYPE type) noexcept;
		CommandAllocatorPool(const CommandAllocatorPool& other) = delete;
		CommandAllocatorPool(CommandAllocatorPool&&) = delete;
		CommandAllocatorPool& operator=(const CommandAllocatorPool& other) = delete;
		CommandAllocatorPool& operator=(CommandAllocatorPool&&) = delete;
		~CommandAllocatorPool() noexcept = default;

		void Initialize(_In_ ID3D12Device* pDevice) noexcept;
		void Destroy();

		HRESULT RequestAllocator(_Out_ ID3D12CommandAllocator** ppAllocator, _In_ UINT64 uCompletedFenceValue) noexcept;
		void DiscardAllocator(_In_ UINT64 uFenceValue, _In_ ID3D12CommandAllocator* pAllocator) noexcept;

		constexpr size_t GetSize() const noexcept;

	private:
		const D3D12_COMMAND_LIST_TYPE m_Type;
		ComPtr<ID3D12Device> m_pDevice;
		std::vector<ID3D12CommandAllocator*> m_AllocatorPool;
		std::queue<std::pair<UINT64, ID3D12CommandAllocator*>> m_ReadyAllocators;
		std::mutex m_AllocatorMutex;
	};

	constexpr size_t CommandAllocatorPool::GetSize() const noexcept
	{
		return m_AllocatorPool.size();
	}
}