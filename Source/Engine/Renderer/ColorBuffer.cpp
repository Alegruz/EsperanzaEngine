#include "Pch.h"

#include "Renderer/ColorBuffer.h"
#include "Renderer/DescriptorHeap.h"

namespace esperanza
{
	ColorBuffer::ColorBuffer() noexcept
		: ColorBuffer(Color(0.0f, 0.0f, 0.0f, 0.0f))
	{
	}

	ColorBuffer::ColorBuffer(Color clearColor) noexcept
		: m_ClearColor(clearColor)
		, m_SrvHandle()
		, m_RtvHandle()
		, m_UavHandle{}
		, m_uNumMipMaps(0)	// number of texture sublevels
		, m_uFragmentCount(1)
		, m_uSampleCount(1)
	{
		m_SrvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_RtvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		for (UINT i = 0; i < ARRAYSIZE(m_UavHandle); ++i)
		{
			m_UavHandle[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}
	}

	HRESULT ColorBuffer::InitializeFromSwapChain(_In_ ID3D12Device* pDevice, _In_ DescriptorAllocator& rtvAllocator, const std::wstring& strName, ID3D12Resource* pBaseResource) noexcept
	{
		HRESULT hr = S_OK;

		hr = associateWithResource(pDevice, strName, pBaseResource, D3D12_RESOURCE_STATE_PRESENT);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Associating with Resource failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		hr = rtvAllocator.Allocate(m_RtvHandle, pDevice, 1);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"RTV Handle Allocation failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		pDevice->CreateRenderTargetView(m_pResource.Get(), nullptr, m_RtvHandle);
	}

	HRESULT ColorBuffer::Initialize(ID3D12Device* pDevice, const std::wstring& strName, uint32_t uWidth, uint32_t uHeight, uint32_t uNumMips, DXGI_FORMAT format) noexcept
	{
		return Initialize(pDevice, strName, uWidth, uHeight, uNumMips, format, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
	}

	HRESULT ColorBuffer::Initialize(ID3D12Device* pDevice, const std::wstring& strName, uint32_t uWidth, uint32_t uHeight, uint32_t uNumMips, DXGI_FORMAT format, D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr) noexcept
	{
		HRESULT hr = S_OK;
		uNumMips = (uNumMips == 0 ? computeNumMips(uWidth, uHeight) : uNumMips);
		D3D12_RESOURCE_FLAGS flags = combineResourceFlags();
		D3D12_RESOURCE_DESC resourceDesc = describeTex2d(uWidth, uHeight, 1, uNumMips, format, flags);

		resourceDesc.SampleDesc.Count = m_uFragmentCount;
		resourceDesc.SampleDesc.Quality = 0;

		D3D12_CLEAR_VALUE clearValue = 
		{
			.Format = format,
			.Color =
			{
				m_ClearColor.GetR(),
				m_ClearColor.GetG(),
				m_ClearColor.GetB(),
				m_ClearColor.GetA(),
			},
		};

		hr = initializeTextureResource(pDevice, strName, resourceDesc, clearValue, vidMemPtr);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Initializing Texture Resources failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		initializeDerivedViews(pDevice, format, 1, uNumMips);

		return hr;
	}

	void ColorBuffer::Initialize(ID3D12Device* pDevice, const std::wstring& strName, uint32_t uWidth, uint32_t uHeight, uint32_t uNumMips, DXGI_FORMAT format, EsramAllocator& allocator) noexcept
	{
		UNREFERENCED_PARAMETER(allocator);
		Initialize(pDevice, strName, uWidth, uHeight, uNumMips, format);
	}

	void ColorBuffer::InitializeArray(ID3D12Device* pDevice, const std::wstring& strName, uint32_t uWidth, uint32_t uHeight, uint32_t uArrayCount, DXGI_FORMAT format) noexcept
	{
		InitializeArray(pDevice, strName, uWidth, uHeight, uArrayCount, format, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
	}

	void ColorBuffer::InitializeArray(ID3D12Device* pDevice, const std::wstring& strName, uint32_t uWidth, uint32_t uHeight, uint32_t uArrayCount, DXGI_FORMAT format, D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr) noexcept
	{
		D3D12_RESOURCE_FLAGS flags = combineResourceFlags();
		D3D12_RESOURCE_DESC resourceDesc = describeTex2d(uWidth, uHeight, uArrayCount, 1, format, flags);

		D3D12_CLEAR_VALUE clearValue = 
		{
			.Format = format,
			.Color = 
			{
				m_ClearColor.GetR(),
				m_ClearColor.GetG(),
				m_ClearColor.GetB(),
				m_ClearColor.GetA(),
			}
		};

		initializeTextureResource(pDevice, strName, resourceDesc, clearValue, vidMemPtr);
		initializeDerivedViews(pDevice, format, uArrayCount, 1);
	}

	HRESULT ColorBuffer::InitializeArray(ID3D12Device* pDevice, const std::wstring& strName, uint32_t uWidth, uint32_t uHeight, uint32_t uArrayCount, DXGI_FORMAT format) noexcept
	{
		return InitializeArray(strName, uWidth, uHeight, uArrayCount, format, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
	}

	HRESULT ColorBuffer::initializeDerivedViews(ID3D12Device* pDevice, DXGI_FORMAT format, uint32_t uArraySize) noexcept
	{
		return initializeDerivedViews(pDevice, format, uArraySize, 1);
	}

	HRESULT ColorBuffer::initializeDerivedViews(ID3D12Device* pDevice, DXGI_FORMAT format, uint32_t uArraySize, uint32_t uNumMips) noexcept
	{
		HRESULT hr = S_OK;
		if (uArraySize != 1 && uNumMips != 1)
		{
			GLOGAS(L"We don't support auto-mips on texture arrays");
			return E_FAIL;
		}

		m_uNumMipMaps = uNumMips - 1;

		D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
		D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};

		RtvDesc.Format = format;
		UavDesc.Format = GetUavFormat(format);
		SrvDesc.Format = format;
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (uArraySize > 1)
		{
			RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			RtvDesc.Texture2DArray.MipSlice = 0;
			RtvDesc.Texture2DArray.FirstArraySlice = 0;
			RtvDesc.Texture2DArray.ArraySize = static_cast<UINT>(uArraySize);

			UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			UavDesc.Texture2DArray.MipSlice = 0;
			UavDesc.Texture2DArray.FirstArraySlice = 0;
			UavDesc.Texture2DArray.ArraySize = static_cast<UINT>(uArraySize);

			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			SrvDesc.Texture2DArray.MipLevels = uNumMips;
			SrvDesc.Texture2DArray.MostDetailedMip = 0;
			SrvDesc.Texture2DArray.FirstArraySlice = 0;
			SrvDesc.Texture2DArray.ArraySize = static_cast<UINT>(uArraySize);
		}
		else if (m_uFragmentCount > 1)
		{
			RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RtvDesc.Texture2D.MipSlice = 0;

			UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			UavDesc.Texture2D.MipSlice = 0;

			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MipLevels = uNumMips;
			SrvDesc.Texture2D.MostDetailedMip = 0;
		}

		if (m_SrvHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			//m_RtvHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			//m_SrvHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		ID3D12Resource* Resource = m_pResource.Get();

		// Create the render target view
		pDevice->CreateRenderTargetView(Resource, &RtvDesc, m_RtvHandle);

		// Create the shader resource view
		pDevice->CreateShaderResourceView(Resource, &SrvDesc, m_SrvHandle);

		if (m_uFragmentCount > 1)
		{
			return E_FAIL;
		}

		// Create the UAVs for each mip level (RWTexture2D)
		for (uint32_t i = 0; i < uNumMips; ++i)
		{
			if (m_UavHandle[i].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			{
				m_UavHandle[i] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			pDevice->CreateUnorderedAccessView(Resource, nullptr, &UavDesc, m_UavHandle[i]);

			UavDesc.Texture2D.MipSlice++;
		}
	}
}