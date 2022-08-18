#include "Pch.h"
#include "Renderer/Renderer.h"

#include "Renderer/CommandListManager.h"
#include "Window/MainWindow.h"

namespace esperanza
{
	Renderer::Renderer() noexcept
		: m_Logger()
		, m_uWidth()
		, m_uHeight()
		, m_pCommandManager(std::make_shared<CommandListManager>())
		, m_Display()
		, m_Viewport()
		, m_ScissorRect()
		, m_pDevice()
		, m_pCommandAllocator()
		, m_pCommandQueue()
		, m_pRootSignature()
		, m_pRtvDescriptorHeap()
		, m_pPipelineState()
		, m_pCommandList()
		, m_uRtvDescriptorSize()
		, m_VertexBuffer()
		, m_VertexBufferView()
		, m_uFrameIndex()
		, m_hFenceEvent()
		, m_pFence()
		, m_uFenceValue()
		, m_bTypedUAVLoadSupport_R11G11B10_FLOAT(FALSE)
		, m_bTypedUAVLoadSupport_R16G16B16A16_FLOAT(FALSE)
	{
	}

	HRESULT Renderer::Initialize(_In_ const MainWindow& window) noexcept
	{
		HRESULT hr = S_OK;

		// https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
		// Initialize the pipeline
		DWORD dwDxgiFactoryFlags = 0;
			// Enable the debug layer
#ifdef _DEBUG
		{
			ComPtr<ID3D12Debug> pDebugInterface;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugInterface))))
			{
				pDebugInterface->EnableDebugLayer();

				// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/GraphicsCore.cpp
				BOOL uUseGpuBasedValidation = TRUE;
				if (uUseGpuBasedValidation)
				{
					ComPtr<ID3D12Debug1> pDebugInterface1;

					if (SUCCEEDED((pDebugInterface.As(&pDebugInterface1))))
					{
						pDebugInterface1->SetEnableGPUBasedValidation(TRUE);
					}
				}
			}
		}

		// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/GraphicsCore.cpp
		ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDxgiInfoQueue))))
		{
			dwDxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

			hr = pDxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
			if (FAILED(hr))
			{
				_com_error err(hr);
				LOGEF(m_Logger, L"Setting Break on Error Severity failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}

			pDxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			if (FAILED(hr))
			{
				_com_error err(hr);
				LOGEF(m_Logger, L"Setting Break on Corruption Severity failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}

			DXGI_INFO_QUEUE_MESSAGE_ID aHide[] =
			{
				80,	/* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */
			};
			DXGI_INFO_QUEUE_FILTER filter =
			{
				.DenyList =
				{
					.NumIDs = ARRAYSIZE(aHide),
					.pIDList = aHide,
				},
			};
			hr = pDxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
			if (FAILED(hr))
			{
				_com_error err(hr);
				LOGEF(m_Logger, L"Adding Storage Filter Entries failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}
		}

		m_Logger.Initialize(Log::eVerbosity::All);
#else
		m_Logger.Initialize(Log::eVerbosity::Error);
#endif

			// Create the device
		ComPtr<IDXGIFactory6> pFactory;
		hr = CreateDXGIFactory2(dwDxgiFactoryFlags, IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
		{
			_com_error err(hr);
			LOGEF(m_Logger, L"Creating DXGI Factory failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		BOOL bUseWarpDevice = FALSE;
		// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/GraphicsCore.cpp
		UINT uDesiredVendor = 0x10DE;	// NVIDIA
		if (uDesiredVendor)
		{
			LOGI(m_Logger, L"Looking for a NVIDIA GPU");
		}

		// Temporary workaround because SetStablePowerState() is crashing
		D3D12EnableExperimentalFeatures(0, nullptr, nullptr, nullptr);

		ComPtr<ID3D12Device> pDevice;
		ComPtr<IDXGIAdapter1> pAdapter;
		BOOL bRequireDxrSupport = FALSE;
		//if (bUseWarpDevice)
		//{
		//	ComPtr<IDXGIAdapter> pWarpAdapter;
		//	hr = pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
		//	if (FAILED(hr))
		//	{
		//		_com_error err(hr);
		//		LOGEF(m_Logger, L"Enumerating Warp Adapter failed with HRESULT code %u, %s", hr, err.ErrorMessage());
		//
		//		return hr;
		//	}
		//
		//	hr = D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
		//	if (FAILED(hr))
		//	{
		//		_com_error err(hr);
		//		LOGEF(m_Logger, L"Creating D3D12 Device failed with HRESULT code %u, %s", hr, err.ErrorMessage());
		//
		//		return hr;
		//	}
		//}
		//else
		if (!bUseWarpDevice)
		{
			// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/GraphicsCore.cpp
			SIZE_T maxSize = 0;

			for (UINT uAdapterIdx = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(uAdapterIdx, &pAdapter); ++uAdapterIdx)
			{
				DXGI_ADAPTER_DESC1 desc;
				pAdapter->GetDesc1(&desc);

				// Is a software adapter?
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					continue;
				}

				// Is this the desired vendor desired?
				if (uDesiredVendor && uDesiredVendor != desc.VendorId)
				{
					continue;
				}

				// Can create a D3D12 device?
				if (FAILED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice))))
				{
					continue;
				}

				// Does support DXR if required?
				if (bRequireDxrSupport && !isDirectXRayTracingSupported(pDevice.Get()))
				{
					continue;
				}

				// By default, search for the adapter with the most memory because that's usually the dGPU
				if (desc.DedicatedVideoMemory < maxSize)
				{
					continue;
				}

				maxSize = desc.DedicatedVideoMemory;

				if (m_pDevice)
				{
					m_pDevice->Release();
				}

				m_pDevice = pDevice.Detach();

				LOGIF(m_Logger, L"Selected GPU: %s (%u MB)", desc.Description, desc.DedicatedVideoMemory >> 20);
			}
			
			//getHardwareAdapter(&pHardwareAdapter, pFactory.Get());
			//hr = D3D12CreateDevice(pHardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
			//if (FAILED(hr))
			//{
			//	_com_error err(hr);
			//	LOGEF(m_Logger, L"Creating D3D12 Device failed with HRESULT code %u, %s", hr, err.ErrorMessage());
			//
			//	return hr;
			//}
		}

		if (bRequireDxrSupport && !m_pDevice)
		{
			LOGE(m_Logger, L"Unable to find a DXR-capable device. Halting.");
			__debugbreak();
		}

		if (!m_pDevice)
		{
			if (bUseWarpDevice)
			{
				LOGIF(m_Logger, L"WARP software adapter requested. Initializing...");
			}
			else
			{
				LOGEF(m_Logger, L"Failed to find a hardware adapter. Falling back to WARP.");
			}

			hr = pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter));
			if (FAILED(hr))
			{
				_com_error err(hr);
				LOGEF(m_Logger, L"Enumerating Warp Adapter failed with HRESULT code %u, %s", hr, err.ErrorMessage());
				
				return hr;
			}
				
			hr = D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
			if (FAILED(hr))
			{
				_com_error err(hr);
				LOGEF(m_Logger, L"Creating D3D12 Device failed with HRESULT code %u, %s", hr, err.ErrorMessage());
				
				return hr;
			}

			m_pDevice = pDevice.Detach();
		}
#if _DEBUG
		else
		{
			BOOL bIsDeveloperModeEnabled = FALSE;

			// Look in the Windows Registry to determine if Developer Mode is enabled
			HKEY hKey;
			LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
			if (result == ERROR_SUCCESS)
			{
				DWORD keyValue, keySize = sizeof(DWORD);
				result = RegQueryValueEx(hKey, L"AllowDevelopmentWithoutDevLicense", 0, NULL, (byte*)&keyValue, &keySize);
				if (result == ERROR_SUCCESS && keyValue == 1)
				{
					bIsDeveloperModeEnabled = TRUE;
				}
				RegCloseKey(hKey);
			}

			if (!bIsDeveloperModeEnabled)
			{
				LOGW(m_Logger, L"Enable Developer Mode on Windows 10 to get consistent profiling results");
			}

			// Prevent the GPU from overclocking or underclocking to get consistent timings
			if (bIsDeveloperModeEnabled)
			{
				m_pDevice->SetStablePowerState(TRUE);
			}
		}

		{
			ComPtr<ID3D12InfoQueue> pInfoQueue;
			if (SUCCEEDED(m_pDevice.As(&pInfoQueue)))
			{
				// Suppress whole categories of messages
				//D3D12_MESSAGE_CATEGORY aCategories[] = {};

				// Suppress messages based on their severity level
				D3D12_MESSAGE_SEVERITY aSeverities[] =
				{
					D3D12_MESSAGE_SEVERITY_INFO
				};

				// Suppress individual messages by their ID
				D3D12_MESSAGE_ID aDenyIds[] =
				{
					// This occurs when there are uninitialized descriptors in a descriptor table, even when a
					// shader does not access the missing descriptors.  I find this is common when switching
					// shader permutations and not wanting to change much code to reorder resources.
					D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

					// Triggered when a shader does not export all color components of a render target, such as
					// when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
					D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

					// This occurs when a descriptor table is unbound even when a shader does not access the missing
					// descriptors.  This is common with a root signature shared between disparate shaders that
					// don't all need the same types of resources.
					D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

					// RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS
					(D3D12_MESSAGE_ID)1008,
				};

				D3D12_INFO_QUEUE_FILTER NewFilter =
				{
					.DenyList =
					{
						// .NumCategories = ARRAYSIZE(aCategories);
						// .pCategoryList = aCategories;
						.NumSeverities = ARRAYSIZE(aSeverities),
						.pSeverityList = aSeverities,
						.NumIDs = ARRAYSIZE(aDenyIds),
						.pIDList = aDenyIds
					}
				};

				pInfoQueue->PushStorageFilter(&NewFilter);
			}
		}
#endif
		// We like to do read-modify-write operations on UAVs during post processing.  To support that, we
		// need to either have the hardware do typed UAV loads of R11G11B10_FLOAT or we need to manually
		// decode an R32_UINT representation of the same buffer.  This code determines if we get the hardware
		// load support.
		D3D12_FEATURE_DATA_D3D12_OPTIONS featureData = {};
		if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureData, sizeof(featureData))))
		{
			if (featureData.TypedUAVLoadAdditionalFormats)
			{
				D3D12_FEATURE_DATA_FORMAT_SUPPORT support =
				{
					.Format = DXGI_FORMAT_R11G11B10_FLOAT, 
					.Support1 = D3D12_FORMAT_SUPPORT1_NONE, 
					.Support2 = D3D12_FORMAT_SUPPORT2_NONE
				};

				if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support))) &&
					(support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
				{
					m_bTypedUAVLoadSupport_R11G11B10_FLOAT = true;
				}

				support.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

				if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support))) &&
					(support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
				{
					m_bTypedUAVLoadSupport_R16G16B16A16_FLOAT = true;
				}
			}
		}

		hr = m_pCommandManager->Initialize(m_pDevice.Get());
		if (FAILED(hr))
		{
			_com_error err(hr);
			LOGEF(m_Logger, L"Initializing command manager failed with HRESULT code %u, %s", hr, err.ErrorMessage());
		
			return hr;
		}
		
		// Initialize Common States
		
		m_Display.Initialize(window, m_pCommandManager);

		return hr;
	}

	void Renderer::Destroy() noexcept
	{
		m_pCommandManager->IdleGpu();
		m_pCommandManager->Destroy();
		
		m_Display.Destroy();

#ifdef _DEBUG
		{
			ComPtr<ID3D12DebugDevice> pDebugInterface;
			if (SUCCEEDED(m_pDevice.As(&pDebugInterface)))
			{
				pDebugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
			}
		}
#endif

		// https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
		// Wait for the GPU to finish
		// Close the event handle

		m_Logger.Destroy();


		m_pFence.Reset();
		m_VertexBuffer.Reset();
		m_pPipelineState.Reset();
		m_pRtvDescriptorHeap.Reset();
		m_pRootSignature.Reset();
		m_pCommandQueue.Reset();
		m_pCommandAllocator.Reset();
		m_pCommandList.Reset();
		m_pCommandManager.reset();
	}

	void Renderer::Update(_In_ FLOAT deltaTime) noexcept
	{
		UNREFERENCED_PARAMETER(deltaTime);
		// https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
		// Modify the constant, vertex, index buffers, and everything else, as necessary
	}

	void Renderer::Render() noexcept
	{
		// https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
		// Populate the command list
			// Reset the command list allocator
			// Reset the command list
			// Set the graphics root signature
			// Set the viewport and scissor rectangles
			// Set a resource barrier, indicating the back buffer is to be used as a render target
			// Record commands into the command list
			// Indicate the back buffer will be used to present after the command list has executed
			// Close the command list to further recording

		// Execute the command list
		// Present the frame
		// Wait for the GPU to finish
	}

	void Renderer::getHardwareAdapter(_Out_ IDXGIAdapter1** ppOutAdapter, _Inout_ IDXGIFactory1* pFactory) noexcept
	{
		getHardwareAdapter(ppOutAdapter, pFactory, FALSE);
	}

	// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloTriangle/DXSample.cpp
	void Renderer::getHardwareAdapter(_Out_ IDXGIAdapter1** ppOutAdapter, _Inout_ IDXGIFactory1* pFactory, _In_ BOOL bRequestHighPerformanceAdapter) noexcept
	{
		*ppOutAdapter = nullptr;

		ComPtr<IDXGIAdapter1> pAdapter;

		ComPtr<IDXGIFactory6> pFactory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&pFactory6))))
		{
			for (
				UINT uAdapterIdx = 0;
				SUCCEEDED(pFactory6->EnumAdapterByGpuPreference(
					uAdapterIdx,
					bRequestHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&pAdapter)
				));
				++uAdapterIdx
				)
			{
				DXGI_ADAPTER_DESC1 desc;
				pAdapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter
					// If you want a software adapter, use warp
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet
				if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		if (!pAdapter.Get())
		{
			for (UINT uAdapterIdx = 0; SUCCEEDED(pFactory->EnumAdapters1(uAdapterIdx, &pAdapter)); ++uAdapterIdx)
			{
				DXGI_ADAPTER_DESC1 desc;
				pAdapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter
					// If you want a software adapter, use warp
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet
				if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		*ppOutAdapter = pAdapter.Detach();
	}

	BOOL Renderer::isDirectXRayTracingSupported(_In_ ID3D12Device* pTestDevice) noexcept
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupport = {};

		if (FAILED(pTestDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupport, sizeof(featureSupport))))
		{
			return FALSE;
		}

		return featureSupport.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
	}
}