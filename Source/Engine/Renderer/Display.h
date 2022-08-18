#pragma once

#include "Pch.h"

namespace esperanza
{
	class CommandListManager;
	class MainWindow;

	class Display final
	{
	public:
		explicit Display() noexcept;
		Display(const Display& other) = delete;
		Display(Display&& other) = delete;
		Display& operator=(const Display& other) = delete;
		Display& operator=(Display&& other) = delete;
		~Display() noexcept = default;

		HRESULT Initialize(_In_ const MainWindow& window, _In_ std::shared_ptr<CommandListManager>& pCommandListManager) noexcept;
		void Destroy() noexcept;

		HRESULT Resize(_In_ UINT uWidth, _In_ UINT uHeight) noexcept;
		void Present() noexcept;

		constexpr UINT GetWidth() const noexcept;
		constexpr UINT GetHeight() const noexcept;
		constexpr BOOL IsHdrOutputEnabled() const noexcept;
		constexpr const UINT64 GetFrameCount() const noexcept;

		constexpr FLOAT GetFrameTime() const noexcept;
		constexpr FLOAT GetFrameRate() const noexcept;

	private:
		enum class eResolution : UINT8
		{
			HD,
			HDP,
			FHD,
			QHD,
			QHDP,
			UHD,
			COUNT,
		};

		enum class eEqaaQuality : UINT8
		{
			EQAA1x1,
			EQAA1x8,
			EQAA1x16,
			COUNT,
		};

		enum class eHdrMode : UINT8
		{
			HDR,
			SDR,
			SIDE_BY_SIDE,
			COUNT,
		};

		enum class eDebugZoomLevel : UINT8
		{
			OFF,
			X2,
			X4,
			X8,
			X16,
			COUNT,
		};

	private:
		//void preparePresentSdr();
		//void preparePresentHdr();
		//void compositeOverlays(GraphicsContext& context);
		static void resolutionToUint(UINT& uOutWidth, UINT& uOutHeight, eResolution resolution) noexcept;
		void setNativeResolution() noexcept;

	private:
		static constexpr const CHAR* RESOLUTION_LABELS[static_cast<size_t>(eResolution::COUNT)] =
		{
			"1280x720",
			"1600x900",
			"1920x1080",
			"2560x1440",
			"3200x1800",
			"3840x2160",
		};

		static constexpr const CHAR* HDR_MODE_LABELS[static_cast<size_t>(eHdrMode::COUNT)] =
		{
			"HDR",
			"SDR",
			"Side-by-Side",
		};

		static constexpr const CHAR* DEBUG_ZOOM_LABELS[static_cast<size_t>(eDebugZoomLevel::COUNT)] =
		{
			"Off",
			"2x Zoom",
			"4x Zoom",
			"8x Zoom",
			"16x Zoom",
		};

		static constexpr const size_t NUM_SWAP_CHAIN_BUFFERS = 3;
		static constexpr const DXGI_FORMAT SWAP_CHAIN_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		//static constexpr const DXGI_FORMAT SWAP_CHAIN_FORMAT = DXGI_FORMAT_R10G10B10A2_UNORM;

	private:
		std::shared_ptr<CommandListManager> m_pCommandListManager;

		UINT m_uNativeWidth;
		UINT m_uNativeHeight;
		UINT m_uDisplayWidth;
		UINT m_uDisplayHeight;
		BOOL m_bIsHdrOutputEnabled;
		FLOAT m_FrameTime;
		UINT64 m_uFrameIndex;
		LARGE_INTEGER m_FrameStartTick;
		LARGE_INTEGER m_FrameStartFrequency;
		BOOL m_bIsVSyncEnabled;
		BOOL m_bIsLimitedTo30Hz;
		BOOL m_bDropRandomFrames;
		eResolution m_NativeResolution;
		FLOAT m_HdrPaperWhite;
		FLOAT m_MaxDisplayLuminance;
		eHdrMode m_HdrDebugMode;
		// ColorBuffer m_PreDisplayBuffer
		// ColorBuffer m_aDisplayPlanes[NUM_SWAP_CHAIN_BUFFERS];
		UINT m_uCurrentBufferIndex;

		ComPtr<IDXGISwapChain1> m_pSwapChain1;
		// RootSignature m_PresentRS;
		// GraphicsPSOs ...


	};
}