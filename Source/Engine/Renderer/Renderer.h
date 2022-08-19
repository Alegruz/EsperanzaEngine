#pragma once

#include "Pch.h"

#include "Renderer/DescriptorHeap.h"
#include "Renderer/Display.h"

namespace esperanza
{
	class CommandListManager;
	class MainWindow;

	class Renderer final
	{
	public:
		explicit Renderer() noexcept;
		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;
		~Renderer() noexcept = default;

		HRESULT Initialize(_In_ const MainWindow& window) noexcept;
		void Destroy() noexcept;
		void Update(_In_ FLOAT deltaTime) noexcept;
		void Render() noexcept;

	private:
		static void getHardwareAdapter(_Out_ IDXGIAdapter1** ppOutAdapter, _Inout_ IDXGIFactory1* pFactory) noexcept;
		static void getHardwareAdapter(_Out_ IDXGIAdapter1** ppOutAdapter, _Inout_ IDXGIFactory1* pFactory, _In_ BOOL bRequestHighPerformanceAdapter) noexcept;
		static BOOL isDirectXRayTracingSupported(_In_ ID3D12Device* pTestDevice) noexcept;

	private:
		Log m_Logger;
		UINT m_uWidth;
		UINT m_uHeight;

		std::shared_ptr<CommandListManager> m_pCommandManager;
		Display m_Display;

		// Pipeline objects
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;
		ComPtr<ID3D12Device> m_pDevice;
		ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
		ComPtr<ID3D12CommandQueue> m_pCommandQueue;
		ComPtr<ID3D12RootSignature> m_pRootSignature;
		ComPtr<ID3D12PipelineState> m_pPipelineState;
		ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
		
		DescriptorAllocator m_pDescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		// App resources
		ComPtr<ID3D12Resource> m_VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

		// Synchronization objects
		UINT m_uFrameIndex;
		HANDLE m_hFenceEvent;
		ComPtr<ID3D12Fence> m_pFence;
		UINT64 m_uFenceValue;

		// Flags
		BOOL m_bTypedUAVLoadSupport_R11G11B10_FLOAT;
		BOOL m_bTypedUAVLoadSupport_R16G16B16A16_FLOAT;
	};
}