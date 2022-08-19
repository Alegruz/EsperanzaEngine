#pragma once

#include "Renderer/GpuResource.h"

namespace esperanza
{
	class EsramAllocator;

	class PixelBuffer : public GpuResource
	{
	public:
		explicit PixelBuffer() noexcept;

		constexpr UINT GetWidth() const noexcept;
		constexpr UINT GetHeight() const noexcept;
		constexpr UINT GetDepth() const noexcept;
		constexpr const DXGI_FORMAT& GetFormat() const noexcept;

		void ExportToFile(const std::wstring& strFilePath) noexcept;

	protected:
		static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format) noexcept;
		static DXGI_FORMAT GetUavFormat(DXGI_FORMAT format) noexcept;
		static DXGI_FORMAT GetDsvFormat(DXGI_FORMAT format) noexcept;
		static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT format) noexcept;
		static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT format) noexcept;
		static size_t GetBytesPerPixel(DXGI_FORMAT format) noexcept;

	protected:
		D3D12_RESOURCE_DESC describeTex2d(UINT uWidth, UINT uHeight, UINT uDepthOrArraySize, UINT uNumMips, DXGI_FORMAT format, UINT uFlags) noexcept;
		HRESULT associateWithResource(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES currentState) noexcept;
		HRESULT initializeTextureResource(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ const D3D12_RESOURCE_DESC& resourceDesc, _In_ D3D12_CLEAR_VALUE clearValue) noexcept;
		HRESULT initializeTextureResource(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ const D3D12_RESOURCE_DESC& resourceDesc, _In_ D3D12_CLEAR_VALUE clearValue, _In_opt_ D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr) noexcept;
		HRESULT initializeTextureResource(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ const D3D12_RESOURCE_DESC& resourceDesc, _In_ D3D12_CLEAR_VALUE clearValue, _In_ EsramAllocator& allocator) noexcept;

	protected:
		UINT m_uWidth;
		UINT m_uHeight;
		UINT m_uArraySize;
		DXGI_FORMAT m_Format;
	};
}