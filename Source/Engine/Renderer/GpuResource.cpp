#include "Pch.h"
#include "GpuResource.h"

namespace esperanza
{
	GpuResource::GpuResource() noexcept
		: m_pResource()
		, m_UsageState(D3D12_RESOURCE_STATE_COMMON)
		, m_TransitioningState((D3D12_RESOURCE_STATES) - 1)
		, m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		, m_uVersionId()
	{
	}

	GpuResource::GpuResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES currentState) noexcept
		: m_pResource(pResource)
		, m_UsageState(currentState)
		, m_TransitioningState((D3D12_RESOURCE_STATES)-1)
		, m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		, m_uVersionId()
	{
	}

	void GpuResource::Destroy() noexcept
	{
		m_pResource.Reset();
		m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		++m_uVersionId;
	}

	ID3D12Resource* GpuResource::operator->() noexcept
	{
		return m_pResource.Get();
	}

	const ID3D12Resource* GpuResource::operator->() const noexcept
	{
		return m_pResource.Get();
	}

	ID3D12Resource* GpuResource::GetResource() noexcept
	{
		return m_pResource.Get();;
	}
	
	const ID3D12Resource* GpuResource::GetResource() const noexcept
	{
		return m_pResource.Get();
	}

	ID3D12Resource** GpuResource::GetAddressOf() noexcept
	{
		return m_pResource.GetAddressOf();
	}

	constexpr D3D12_GPU_VIRTUAL_ADDRESS GpuResource::GetGpuVirtualAddress() const noexcept
	{
		return m_GpuVirtualAddress;
	}

	constexpr UINT GpuResource::GetVersionId() const noexcept
	{
		return m_uVersionId;
	}
}