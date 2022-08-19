#pragma once

#include "Renderer/Color.h"
#include "Renderer/PixelBuffer.h"

namespace esperanza
{
    class DescriptorAllocator;
	class EsramAllocator;

	class ColorBuffer final : public PixelBuffer
	{
	public:
		explicit ColorBuffer() noexcept;
		explicit ColorBuffer(_In_ Color clearColor) noexcept;
		ColorBuffer(const ColorBuffer& other) = delete;
		ColorBuffer(ColorBuffer&& other) = delete;
		ColorBuffer& operator=(const ColorBuffer& other) = delete;
		ColorBuffer& operator=(ColorBuffer&& other) = delete;
		~ColorBuffer() noexcept = default;

        // Initialize a color buffer from a swap chain buffer.  Unordered access is restricted.
        HRESULT InitializeFromSwapChain(_In_ ID3D12Device* pDevice, _In_ DescriptorAllocator& rtvAllocator, _In_ const std::wstring& strName, _In_ ID3D12Resource* pBaseResource) noexcept;

        // Initialize a color buffer.  If an address is supplied, memory will not be allocated.
        // The vmem address allows you to alias buffers (which can be especially useful for
        // reusing ESRAM across a frame.)
        HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ uint32_t uWidth, _In_  uint32_t uHeight, _In_ uint32_t uNumMips,
            _In_ DXGI_FORMAT format) noexcept;

        HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ uint32_t uWidth, _In_  uint32_t uHeight, _In_ uint32_t uNumMips,
            _In_ DXGI_FORMAT format, _In_ D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr) noexcept;

        // Initialize a color buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
        // this functions the same as Initialize() without a video address.
        HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ uint32_t uWidth, _In_ uint32_t uHeight, _In_ uint32_t uNumMips,
            _In_ DXGI_FORMAT format, _In_ EsramAllocator& allocator) noexcept;

        // Initialize a color buffer.  If an address is supplied, memory will not be allocated.
        // The vmem address allows you to alias buffers (which can be especially useful for
        // reusing ESRAM across a frame.)
        HRESULT InitializeArray(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ uint32_t uWidth, _In_ uint32_t uHeight, _In_ uint32_t uArrayCount,
            _In_ DXGI_FORMAT format) noexcept;

        HRESULT InitializeArray(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ uint32_t uWidth, _In_ uint32_t uHeight, _In_ uint32_t uArrayCount,
            _In_ DXGI_FORMAT format, _In_ D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr) noexcept;

        // Initialize a color buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
        // this functions the same as Initialize() without a video address.
        HRESULT InitializeArray(_In_ ID3D12Device* pDevice, _In_ const std::wstring& strName, _In_ uint32_t uWidth, _In_ uint32_t uHeight, _In_ uint32_t uArrayCount,
            _In_ DXGI_FORMAT format, _In_ EsramAllocator& allocator) noexcept;

        // Get pre-created CPU-visible descriptor handles
        constexpr const D3D12_CPU_DESCRIPTOR_HANDLE& GetSrv() const noexcept;
        constexpr const D3D12_CPU_DESCRIPTOR_HANDLE& GetRtv() const noexcept;
        constexpr const D3D12_CPU_DESCRIPTOR_HANDLE& GetUav() const noexcept;

        constexpr void SetClearColor(Color ClearColor) noexcept;

        constexpr void SetMsaaMode(uint32_t NumColorSamples, uint32_t NumCoverageSamples) noexcept;

        constexpr const Color& GetClearColor(void) const noexcept;

        // This will work for all texture sizes, but it's recommended for speed and quality
        // that you use dimensions with powers of two (but not necessarily square.)  Pass
        // 0 for ArrayCount to reserve space for mips at creation time.
        //void GenerateMipMaps(CommandContext& Context);

    protected:
        // Compute the number of texture levels needed to reduce to 1x1.  This uses
        // _BitScanReverse to find the highest set bit.  Each dimension reduces by
        // half and truncates bits.  The dimension 256 (0x100) has 9 mip levels, same
        // as the dimension 511 (0x1FF).
        static constexpr uint32_t computeNumMips(uint32_t uWidth, uint32_t uHeight) noexcept;

    protected:
        constexpr D3D12_RESOURCE_FLAGS combineResourceFlags(void) const noexcept;

        HRESULT initializeDerivedViews(ID3D12Device* pDevice, DXGI_FORMAT format, uint32_t uArraySize) noexcept;
        HRESULT initializeDerivedViews(ID3D12Device* pDevice, DXGI_FORMAT format, uint32_t uArraySize, uint32_t uNumMips) noexcept;

    protected:
        Color m_ClearColor;
        D3D12_CPU_DESCRIPTOR_HANDLE m_SrvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE m_RtvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE m_UavHandle[12];
        uint32_t m_uNumMipMaps; // number of texture sublevels
        uint32_t m_uFragmentCount;
        uint32_t m_uSampleCount;
	};
}