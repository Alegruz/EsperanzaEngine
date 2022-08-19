#include "Pch.h"

#include "Renderer/PixelBuffer.h"

#include <fstream>

namespace esperanza
{
    PixelBuffer::PixelBuffer() noexcept
        : m_uWidth(0)
        , m_uHeight(0)
        , m_uArraySize(0)
        , m_Format(DXGI_FORMAT_UNKNOWN)
    {
    }

    constexpr UINT PixelBuffer::GetWidth() const noexcept
    {
        return m_uWidth;
    }

    constexpr UINT PixelBuffer::GetHeight() const noexcept
    {
        return m_uHeight;
    }

    constexpr UINT PixelBuffer::GetDepth() const noexcept
    {
        return m_uArraySize;
    }

    constexpr const DXGI_FORMAT& PixelBuffer::GetFormat() const noexcept
    {
        return m_Format;
    }

    void PixelBuffer::ExportToFile(const std::wstring& strFilePath) noexcept
    {
        // This very short command list only issues one API call and will be synchronized so we can immediately read
        // the buffer contents.
        //ReadbackBuffer TempBuffer;
        //CommandContext& Context = CommandContext::Begin(L"Copy texture to memory");
        //uint32_t RowPitch = Context.ReadbackTexture(TempBuffer, *this);
        //Context.Finish(true);

        // Retrieve a CPU-visible pointer to the buffer memory.  Map the whole range for reading.
        //void* Memory = TempBuffer.Map();

        // Open the file and write the header followed by the texel data.
        std::ofstream outFile(strFilePath, std::ios::out | std::ios::binary);
        outFile.write(reinterpret_cast<const char*>(&m_Format), 4);
        //outFile.write(reinterpret_cast<const char*>(&RowPitch), 4);
        outFile.write(reinterpret_cast<const char*>(&m_uWidth), 4);
        outFile.write(reinterpret_cast<const char*>(&m_uHeight), 4);
        //outFile.write((const char*)Memory, TempBuffer.GetBufferSize());
        outFile.close();

        // No values were written to the buffer, so use a null range when unmapping.
        //TempBuffer.Unmap();
    }

    DXGI_FORMAT PixelBuffer::GetBaseFormat(DXGI_FORMAT format) noexcept
	{
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;

        case DXGI_FORMAT_B8G8R8A8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;

        case DXGI_FORMAT_B8G8R8X8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8X8_TYPELESS;

            // 32-bit Z w/ Stencil
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return DXGI_FORMAT_R32G8X24_TYPELESS;

            // No Stencil
        case DXGI_FORMAT_R32_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT:
            return DXGI_FORMAT_R32_TYPELESS;

            // 24-bit Z
        case DXGI_FORMAT_R24G8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return DXGI_FORMAT_R24G8_TYPELESS;

            // 16-bit Z w/o Stencil
        case DXGI_FORMAT_R16_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D16_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16_UNORM:
            return DXGI_FORMAT_R16_TYPELESS;

        default:
            return format;
        }
	}

    DXGI_FORMAT PixelBuffer::GetUavFormat(DXGI_FORMAT format) noexcept
    {
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM;

        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM;

        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8X8_UNORM;

        case DXGI_FORMAT_R32_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT:
            return DXGI_FORMAT_R32_FLOAT;

#ifdef _DEBUG
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R24G8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_D16_UNORM:

            GLOGAS(L"Requested a UAV Format for a depth stencil Format.");
            assert(false);
#endif

        default:
            return format;
        }
    }

    DXGI_FORMAT PixelBuffer::GetDsvFormat(DXGI_FORMAT format) noexcept
    {
        switch (format)
        {
            // 32-bit Z w/ Stencil
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

            // No Stencil
        case DXGI_FORMAT_R32_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;

            // 24-bit Z
        case DXGI_FORMAT_R24G8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;

            // 16-bit Z w/o Stencil
        case DXGI_FORMAT_R16_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D16_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16_UNORM:
            return DXGI_FORMAT_D16_UNORM;

        default:
            return format;
        }
    }

    DXGI_FORMAT PixelBuffer::GetDepthFormat(DXGI_FORMAT format) noexcept
    {
        switch (format)
        {
            // 32-bit Z w/ Stencil
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

            // No Stencil
        case DXGI_FORMAT_R32_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT:   
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT:
            return DXGI_FORMAT_R32_FLOAT;

            // 24-bit Z
        case DXGI_FORMAT_R24G8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

            // 16-bit Z w/o Stencil
        case DXGI_FORMAT_R16_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D16_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16_UNORM:
            return DXGI_FORMAT_R16_UNORM;

        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    DXGI_FORMAT PixelBuffer::GetStencilFormat(DXGI_FORMAT format) noexcept
    {
        switch (format)
        {
            // 32-bit Z w/ Stencil
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

            // 24-bit Z
        case DXGI_FORMAT_R24G8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    size_t PixelBuffer::GetBytesPerPixel(DXGI_FORMAT format) noexcept
    {
        switch (format)
        {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32B32A32_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 16;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32B32_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32B32_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32B32_SINT:
            return 12;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16B16A16_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16B16A16_SNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16B16A16_SINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32G32_SINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return 8;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R10G10B10A2_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R11G11B10_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_SNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8B8A8_SINT:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16_SNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16G16_SINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D32_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R32_SINT:
            [[fallthrough]];
        case DXGI_FORMAT_R24G8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return 4;

        case DXGI_FORMAT_R8G8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8_SNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R8G8_SINT:
            [[fallthrough]];
        case DXGI_FORMAT_R16_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R16_FLOAT:
            [[fallthrough]];
        case DXGI_FORMAT_D16_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R16_SNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R16_SINT:
            [[fallthrough]];
        case DXGI_FORMAT_B5G6R5_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_A8P8:
            [[fallthrough]];
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 2;

        case DXGI_FORMAT_R8_TYPELESS:
            [[fallthrough]];
        case DXGI_FORMAT_R8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R8_UINT:
            [[fallthrough]];
        case DXGI_FORMAT_R8_SNORM:
            [[fallthrough]];
        case DXGI_FORMAT_R8_SINT:
            [[fallthrough]];
        case DXGI_FORMAT_A8_UNORM:
            [[fallthrough]];
        case DXGI_FORMAT_P8:
            return 1;

        default:
            return 0;
        }
    }

    D3D12_RESOURCE_DESC PixelBuffer::describeTex2d(UINT uWidth, UINT uHeight, UINT uDepthOrArraySize, UINT uNumMips, DXGI_FORMAT format, UINT uFlags) noexcept
    {
        m_uWidth = uWidth;
        m_uHeight = uHeight;
        m_uArraySize = uDepthOrArraySize;
        m_Format = format;

        D3D12_RESOURCE_DESC desc =
        {
            .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0,
            .Width = uWidth,
            .Height = uHeight,
            .DepthOrArraySize = static_cast<UINT16>(uDepthOrArraySize),
            .MipLevels = static_cast<UINT16>(uNumMips),
            .Format = GetBaseFormat(format),
            .SampleDesc =
            {
                .Count = 1,
                .Quality = 0,
            },
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = static_cast<D3D12_RESOURCE_FLAGS>(uFlags),
        };

        return desc;
    }

    HRESULT PixelBuffer::associateWithResource(ID3D12Device* pDevice, const std::wstring& strName, ID3D12Resource* pResource, D3D12_RESOURCE_STATES currentState) noexcept
    {
        HRESULT hr = S_OK;

        UNREFERENCED_PARAMETER(pDevice);

        if (!pResource)
        {
            GLOGAS(L"Resource is null!");

            return E_FAIL;
        }

        D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();

        m_pResource.Attach(pResource);
        m_UsageState = currentState;

        m_uWidth = static_cast<UINT>(resourceDesc.Width);
        m_uHeight = resourceDesc.Height;
        m_uArraySize = resourceDesc.DepthOrArraySize;
        m_Format = resourceDesc.Format;

#ifdef _DEBUG
        m_pResource->SetName(strName.c_str());
#else
        UNREFERENCED_PARAMETER(strName);
#endif

        return hr;
    }

    HRESULT PixelBuffer::createTextureResource(ID3D12Device* pDevice, const std::wstring& strName, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_CLEAR_VALUE clearValue) noexcept
    {
        return createTextureResource(pDevice, strName, resourceDesc, clearValue, D3D12_GPU_VIRTUAL_ADDRESS_NULL);
    }

    HRESULT PixelBuffer::createTextureResource(ID3D12Device* pDevice, const std::wstring& strName, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_CLEAR_VALUE clearValue, D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr) noexcept
    {
        HRESULT hr = S_OK;
        Destroy();

        UNREFERENCED_PARAMETER(vidMemPtr);

        {
            CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

            hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_pResource));
            if (FAILED(hr))
            {
                _com_error err(hr);
                GLOGEF(L"Creating Comitted Resource failed with HRESULT code %u, %s", hr, err.ErrorMessage());

                return hr;
            }
        }

        m_UsageState = D3D12_RESOURCE_STATE_COMMON;
        m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;

#ifdef _DEBUG
        m_pResource->SetName(strName.c_str());
#else
        UNREFERENCED_PARAMETER(strName);
#endif

        return hr;
    }

    HRESULT PixelBuffer::createTextureResource(ID3D12Device* pDevice, const std::wstring& strName, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_CLEAR_VALUE clearValue, EsramAllocator& allocator) noexcept
    {
        HRESULT hr = S_OK;

        UNREFERENCED_PARAMETER(allocator);

        createTextureResource(pDevice, strName, resourceDesc, clearValue);

        return hr;
    }
}