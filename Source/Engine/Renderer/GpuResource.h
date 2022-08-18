#pragma once

#include "Pch.h"

namespace esperanza
{
	class GpuResource
	{
		friend class CommandContext;
		friend class GraphicsContext;
		friend class ComputeContext;

	public:
		explicit GpuResource() noexcept;
		explicit GpuResource(_In_ ID3D12Resource* pResource, _In_ D3D12_RESOURCE_STATES currentState) noexcept;
		GpuResource(const GpuResource& other) = delete;
		GpuResource(GpuResource&& other) = delete;
		GpuResource& operator=(const GpuResource& other) = delete;
		GpuResource& operator=(GpuResource&& other) = delete;
		~GpuResource() noexcept = default;

		virtual void Destroy() noexcept;

		ID3D12Resource* operator->() noexcept;
		const ID3D12Resource* operator->() const noexcept;

		ID3D12Resource* GetResource() noexcept;
		const ID3D12Resource* GetResource() const noexcept;
		ID3D12Resource** GetAddressOf() noexcept;
		constexpr D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const noexcept;
		constexpr UINT GetVersionId() const noexcept;

	protected:
		ComPtr<ID3D12Resource> m_pResource;
		D3D12_RESOURCE_STATES m_UsageState;
		D3D12_RESOURCE_STATES m_TransitioningState;
		D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;

		// Used to identify when a resource changes so descriptors can be copied etc.
		UINT m_uVersionId;
	};
}