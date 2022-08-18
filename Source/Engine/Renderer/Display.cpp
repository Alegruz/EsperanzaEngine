#include "Pch.h"
#include "Renderer/Display.h"

#include "Renderer/CommandListManager.h"
#include "Window/MainWindow.h"

// This macro determines whether to detect if there is an HDR display and enable HDR10 output.
// Currently, with HDR display enabled, the pixel magnfication functionality is broken.
#define CONDITIONALLY_ENABLE_HDR_OUTPUT (1)

namespace esperanza
{
	Display::Display() noexcept
		: m_uNativeWidth(0)
		, m_uNativeHeight(0)
		, m_uDisplayWidth(1920)
		, m_uDisplayHeight(1080)
		, m_bIsHdrOutputEnabled(FALSE)
		, m_FrameTime(0)
		, m_uFrameIndex(0)
		, m_FrameStartTick()
		, m_FrameStartFrequency()
		, m_bIsVSyncEnabled(TRUE)
		, m_bIsLimitedTo30Hz(FALSE)
		, m_bDropRandomFrames(FALSE)
		, m_NativeResolution(eResolution::FHD)
		, m_HdrPaperWhite(200.0f)			// 200, 100, 500, 50
		, m_MaxDisplayLuminance(1000.0f)	// 1000, 500, 10000, 100
		, m_HdrDebugMode(eHdrMode::HDR)
		//, m_PreDisplayBuffer()
		// m_aDisplayPlanes{}
		, m_uCurrentBufferIndex(0)
		, m_pSwapChain1()
	{
	}

	HRESULT Display::Initialize(const MainWindow& window, std::shared_ptr<CommandListManager>& pCommandListManager) noexcept
	{
		HRESULT hr = S_OK;
		if (m_pSwapChain1)
		{
			GLOGE(L"Display has already been initialized");
		}

		ComPtr<IDXGIFactory4> pDxgiFactory;
		hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&pDxgiFactory));
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Creating DXGI Factory failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
		{
			.Width = m_uDisplayWidth,
			.Height = m_uDisplayHeight,
			.Format = SWAP_CHAIN_FORMAT,
			.SampleDesc =
			{
				.Count = 1,
				.Quality = 0,
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = NUM_SWAP_CHAIN_BUFFERS,
			.Scaling = DXGI_SCALING_NONE,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_IGNORE,
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenSwapChainDesc =
		{
			.Windowed = TRUE,
		};

		m_pCommandListManager = pCommandListManager;

		hr = pDxgiFactory->CreateSwapChainForHwnd(
			m_pCommandListManager->GetCommandQueue(),
			window.GetWindowHandle(),
			&swapChainDesc,
			&fullscreenSwapChainDesc,
			nullptr,
			&m_pSwapChain1
		);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Creating Swap Chain failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

#if CONDITIONALLY_ENABLE_HDR_OUTPUT
		{
			IDXGISwapChain4* pSwapChain = static_cast<IDXGISwapChain4*>(m_pSwapChain1.Get());
			ComPtr<IDXGIOutput> pOutput;
			ComPtr<IDXGIOutput6> pOutput6;
			DXGI_OUTPUT_DESC1 outputDesc;
			UINT uColorSpaceSupport;

			// Query support for ST.2084 on the display and set the color space accordingly
			if (SUCCEEDED(pSwapChain->GetContainingOutput(&pOutput)) && SUCCEEDED(pOutput.As(&pOutput6)) &&
				SUCCEEDED(pOutput6->GetDesc1(&outputDesc)) && outputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 &&
				SUCCEEDED(pSwapChain->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &uColorSpaceSupport)) &&
				(uColorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) &&
				SUCCEEDED(pSwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)))
			{
				m_bIsHdrOutputEnabled = TRUE;
			}
		}
#endif // End CONDITIONALLY_ENABLE_HDR_OUTPUT

		for (uint32_t i = 0; i < NUM_SWAP_CHAIN_BUFFERS; ++i)
		{
			ComPtr<ID3D12Resource> pDisplayPlane;
			hr = m_pSwapChain1->GetBuffer(i, IID_PPV_ARGS(&pDisplayPlane));
			if (FAILED(hr))
			{
				_com_error err(hr);
				GLOGEF(L"Getting swap chain buffer failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}

			//g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
		}

		// Set PSOs

		setNativeResolution();

		return hr;
	}

	void Display::Destroy() noexcept
	{
		m_pSwapChain1->SetFullscreenState(FALSE, nullptr);

		for (UINT i = 0; i < NUM_SWAP_CHAIN_BUFFERS; ++i)
		{
			// destroy display plane
		}

		// destroy pre display plane
	}

	HRESULT Display::Resize(UINT uWidth, UINT uHeight) noexcept
	{
		HRESULT hr = S_OK;
		m_pCommandListManager->IdleGpu();

		m_uDisplayWidth = uWidth;
		m_uDisplayHeight = uHeight;

		GLOGIF(L"Changing display resolution to %ux%u", uWidth, uHeight);

		//g_PreDisplayBuffer.Create(L"PreDisplay Buffer", width, height, 1, SwapChainFormat);
		//
		//for (uint32_t i = 0; i < NUM_SWAP_CHAIN_BUFFERS; ++i)
		//{
		//	g_DisplayPlane[i].Destroy();
		//}

		if (!m_pSwapChain1)
		{
			GLOGE(L"Swap Chain is null!");

			return E_FAIL;
		}
		
		hr = m_pSwapChain1->ResizeBuffers(NUM_SWAP_CHAIN_BUFFERS, uWidth, uHeight, SWAP_CHAIN_FORMAT, 0);
		if (FAILED(hr))
		{
			_com_error err(hr);
			GLOGEF(L"Resizing swap chain buffers failed with HRESULT code %u, %s", hr, err.ErrorMessage());

			return hr;
		}

		for (uint32_t i = 0; i < NUM_SWAP_CHAIN_BUFFERS; ++i)
		{
			ComPtr<ID3D12Resource> pDisplayPlane;

			hr = m_pSwapChain1->GetBuffer(i, IID_PPV_ARGS(&pDisplayPlane));
			if (FAILED(hr))
			{
				_com_error err(hr);
				GLOGEF(L"Getting swap chain buffer failed with HRESULT code %u, %s", hr, err.ErrorMessage());

				return hr;
			}

			//g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", pDisplayPlane.Detach());
		}

		m_uCurrentBufferIndex = 0;

		m_pCommandListManager->IdleGpu();

		//ResizeDisplayDependentBuffers(g_NativeWidth, g_NativeHeight);

		return hr;
	}

	void Display::Present() noexcept
	{
		if (m_bIsHdrOutputEnabled)
		{

		}
		else
		{

		}

		UINT uPresentInterval = m_bIsVSyncEnabled ? std::min(4u, static_cast<UINT>(std::roundf(m_FrameTime * 60.0f))) : 0;

		m_pSwapChain1->Present(uPresentInterval, 0);

		m_uCurrentBufferIndex = (m_uCurrentBufferIndex + 1) % NUM_SWAP_CHAIN_BUFFERS;

		LARGE_INTEGER currentTick;
		LARGE_INTEGER currentFrequency;
		QueryPerformanceCounter(&currentTick);
		QueryPerformanceFrequency(&currentFrequency);

		if (m_bIsVSyncEnabled)
		{
			m_FrameTime = (m_bIsLimitedTo30Hz ? 2.0f : 1.0f) / 60.0f;

			if (m_bDropRandomFrames)
			{
				if (!(std::rand() % 50))
				{
					m_FrameTime += (1.0f / 60.0f);
				}
			}
		}
		else
		{
			LARGE_INTEGER elapsedMicroseconds;
			elapsedMicroseconds.QuadPart = currentTick.QuadPart - m_FrameStartTick.QuadPart;
			elapsedMicroseconds.QuadPart *= 1000000;
			elapsedMicroseconds.QuadPart /= m_FrameStartFrequency.QuadPart;
			m_FrameTime = static_cast<FLOAT>(elapsedMicroseconds.QuadPart) / 1000000.0f;
		}

		m_FrameStartTick = currentTick;
		m_FrameStartFrequency = currentFrequency;

		++m_uFrameIndex;

		// Update temporal effects

		setNativeResolution();
	}

	constexpr UINT Display::GetWidth() const noexcept
	{
		return m_uDisplayWidth;
	}

	constexpr UINT Display::GetHeight() const noexcept
	{
		return m_uDisplayHeight;
	}

	constexpr BOOL Display::IsHdrOutputEnabled() const noexcept
	{
		return m_bIsHdrOutputEnabled;
	}

	constexpr const UINT64 Display::GetFrameCount() const noexcept
	{
		return m_uFrameIndex;
	}

	constexpr FLOAT Display::GetFrameTime() const noexcept
	{
		return m_FrameTime;
	}

	constexpr FLOAT Display::GetFrameRate() const noexcept
	{
		return m_FrameTime == 0.0f ? 0.0f : 1.0f / m_FrameTime;
	}

	void Display::resolutionToUint(UINT& uOutWidth, UINT& uOutHeight, eResolution resolution) noexcept
	{
		switch (resolution)
		{
		case esperanza::Display::eResolution::HD:
		{
			uOutWidth = 1280;
			uOutHeight = 720;
		}
			break;
		case esperanza::Display::eResolution::HDP:
		{
			uOutWidth = 1600;
			uOutHeight = 900;
		}
			break;
		case esperanza::Display::eResolution::FHD:
		{
			uOutWidth = 1920;
			uOutHeight = 1080;
		}
			break;
		case esperanza::Display::eResolution::QHD:
		{
			uOutWidth = 2560;
			uOutHeight = 1440;
		}
			break;
		case esperanza::Display::eResolution::QHDP:
		{
			uOutWidth = 3200;
			uOutHeight = 1800;
		}
			break;
		case esperanza::Display::eResolution::UHD:
		{
			uOutWidth = 3840;
			uOutHeight = 2160;
		}
			break;
		case esperanza::Display::eResolution::COUNT:
			[[fallthrough]];
		default:
			GLOGE(L"Resolution enumeration invalid value!");
			assert(false);
			break;
		}
	}

	void Display::setNativeResolution() noexcept
	{
		UINT uNativeWidth = 0;
		UINT uNativeHeight = 0;

		resolutionToUint(uNativeWidth, uNativeHeight, m_NativeResolution);

		if (m_uNativeWidth == uNativeWidth && m_uNativeHeight == uNativeHeight)
		{
			return;
		}
		GLOGIF(L"Changing native resolution to %ux%u", uNativeWidth, uNativeHeight);

		m_uNativeWidth = uNativeWidth;
		m_uNativeHeight = uNativeHeight;

		m_pCommandListManager->IdleGpu();

		// InitializeRenderingBuffers
	}
}