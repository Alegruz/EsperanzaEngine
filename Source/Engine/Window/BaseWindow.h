	#pragma once

#include "Pch.h"

namespace esperanza
{
	template <class DerivedType>
	class BaseWindow
	{
	public:
		static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

	public:
		explicit constexpr BaseWindow() noexcept;
		BaseWindow(const BaseWindow& other) = delete;
		BaseWindow(BaseWindow&& other) = delete;
		BaseWindow& operator=(const BaseWindow& other) = delete;
		BaseWindow& operator=(BaseWindow&& other) = delete;
		virtual ~BaseWindow() noexcept = default;

		virtual HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) noexcept = 0;
		virtual constexpr PCWSTR GetWindowClassName() const noexcept = 0;
		virtual LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) noexcept = 0;

		void SetCallbackToWindowMessage(UINT uWindowMessage, void (*pFunction)(WPARAM, LPARAM, ULONGLONG), ULONGLONG uParam) noexcept;

		constexpr HWND GetWindowHandle() const noexcept;
		constexpr BOOL IsActive() const noexcept;
		constexpr BOOL IsWindowed() const noexcept;
		constexpr UINT GetWidth() const noexcept;
		constexpr UINT GetHeight() const noexcept;
		
	protected:
		virtual HRESULT initialize(
			_In_ HINSTANCE hInstance,
			_In_ INT nCmdShow,
			_In_ PCWSTR pszWindowName,
			_In_ DWORD dwStyle
		) noexcept;

		virtual HRESULT initialize(
			_In_ HINSTANCE hInstance,
			_In_ INT nCmdShow,
			_In_ PCWSTR pszWindowName,
			_In_ DWORD dwStyle,
			_In_opt_ INT x,
			_In_opt_ INT y
		) noexcept;

		virtual HRESULT initialize(
			_In_ HINSTANCE hInstance,
			_In_ INT nCmdShow,
			_In_ PCWSTR pszWindowName,
			_In_ DWORD dwStyle,
			_In_opt_ INT x,
			_In_opt_ INT y,
			_In_opt_ INT nWidth,
			_In_opt_ INT nHeight
		) noexcept;

		virtual HRESULT initialize(
			_In_ HINSTANCE hInstance,
			_In_ INT nCmdShow,
			_In_ PCWSTR pszWindowName,
			_In_ DWORD dwStyle,
			_In_opt_ INT x,
			_In_opt_ INT y,
			_In_opt_ INT nWidth,
			_In_opt_ INT nHeight,
			_In_opt_ HWND hWndParent
		) noexcept;

		virtual HRESULT initialize(
			_In_ HINSTANCE hInstance,
			_In_ INT nCmdShow,
			_In_ PCWSTR pszWindowName,
			_In_ DWORD dwStyle,
			_In_opt_ INT x,
			_In_opt_ INT y,
			_In_opt_ INT nWidth,
			_In_opt_ INT nHeight,
			_In_opt_ HWND hWndParent,
			_In_opt_ HMENU hMenu
		) noexcept;

	protected:
		HINSTANCE m_hInstance;
		HWND m_hWnd;
		LPCWSTR m_lpszName;
		BOOL m_bIsWindowed;
		RECT m_rcWindow;
		RECT m_rcDesktop;
		UINT m_uColorDepth;
		BOOL m_bIsActive;
		std::unordered_map<UINT, std::pair<void(*)(WPARAM, LPARAM, ULONGLONG), ULONGLONG>> m_Callbacks;
	};

	template<class DerivedType>
	inline LRESULT BaseWindow<DerivedType>::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DerivedType* pThis = nullptr;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			pThis = static_cast<DerivedType*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

			pThis->m_hWnd = hWnd;
		}
		else
		{
			pThis = reinterpret_cast<DerivedType*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	template<class DerivedType>
	inline constexpr BaseWindow<DerivedType>::BaseWindow() noexcept
		: m_hInstance()
		, m_hWnd()
		, m_lpszName()
		, m_bIsWindowed()
		, m_rcWindow()
		, m_rcDesktop()
		, m_uColorDepth()
		, m_bIsActive(FALSE)
		, m_Callbacks()
	{
	}

	template<class DerivedType>
	inline void BaseWindow<DerivedType>::SetCallbackToWindowMessage(UINT uWindowMessage, void (*pFunction)(WPARAM, LPARAM, ULONGLONG), ULONGLONG uParam) noexcept
	{
		m_Callbacks[uWindowMessage] = { pFunction, uParam };
	}

	template<class DerivedType>
	inline constexpr HWND BaseWindow<DerivedType>::GetWindowHandle() const noexcept
	{
		return m_hWnd;
	}

	template<class DerivedType>
	inline constexpr BOOL BaseWindow<DerivedType>::IsActive() const noexcept
	{
		return m_bIsActive;
	}

	template<class DerivedType>
	inline constexpr BOOL BaseWindow<DerivedType>::IsWindowed() const noexcept
	{
		return m_bIsWindowed;
	}

	template<class DerivedType>
	inline constexpr UINT BaseWindow<DerivedType>::GetWidth() const noexcept
	{
		return m_rcWindow.right - m_rcWindow.left;
	}

	template<class DerivedType>
	inline constexpr UINT BaseWindow<DerivedType>::GetHeight() const noexcept
	{
		return m_rcWindow.bottom - m_rcWindow.top;
	}

	template<class DerivedType>
	inline HRESULT BaseWindow<DerivedType>::initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName, DWORD dwStyle) noexcept
	{
		return initialize(hInstance, nCmdShow, pszWindowName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT);
	}

	template<class DerivedType>
	inline HRESULT BaseWindow<DerivedType>::initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName, DWORD dwStyle, INT x, INT y) noexcept
	{
		return initialize(hInstance, nCmdShow, pszWindowName, dwStyle, x, y, CW_USEDEFAULT, CW_USEDEFAULT);
	}

	template<class DerivedType>
	inline HRESULT BaseWindow<DerivedType>::initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight) noexcept
	{
		return initialize(hInstance, nCmdShow, pszWindowName, dwStyle, x, y, nWidth, nHeight, NULL);
	}

	template<class DerivedType>
	inline HRESULT BaseWindow<DerivedType>::initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight, HWND hWndParent) noexcept
	{
		return initialize(hInstance, nCmdShow, pszWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, NULL);
	}

	template<class DerivedType>
	inline HRESULT BaseWindow<DerivedType>::initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu) noexcept
	{
		// Register the window class
		WNDCLASSEX wcex =
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = WindowProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = hInstance,
			.hIcon = LoadIcon(hInstance, reinterpret_cast<LPCWSTR>(IDI_ESPERANZA)),
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
			.lpszMenuName = nullptr,
			.lpszClassName = GetWindowClassName(),
			.hIconSm = LoadIcon(hInstance, reinterpret_cast<LPCWSTR>(IDI_ESPERANZA))
		};

		if (!RegisterClassEx(&wcex))
		{
			DWORD dwError = GetLastError();

			MessageBox(NULL, L"Call to RegisterEx failed!", ENGINE_NAME, 0u);

			if (dwError != ERROR_CLASS_ALREADY_EXISTS)
			{
				return HRESULT_FROM_WIN32(dwError);
			}

			return E_FAIL;
		}

		// Create window
		m_hInstance = hInstance;
		m_lpszName = pszWindowName;
		m_bIsWindowed = TRUE;

		m_rcWindow =
		{
			.left = 0,
			.top = 0,
			.right = nWidth,
			.bottom = nHeight
		};
		AdjustWindowRect(&m_rcWindow, WS_OVERLAPPEDWINDOW, FALSE);
		m_rcDesktop =
		{
			.left = 0,
			.top = 0,
			.right = GetSystemMetrics(SM_CXSCREEN),
			.bottom = GetSystemMetrics(SM_CYSCREEN),
		};

		m_hWnd = CreateWindowEx(
			0u,
			GetWindowClassName(),
			m_lpszName,
			dwStyle,
			x, y, m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top,
			hWndParent,
			hMenu,
			m_hInstance,
			this
		);

		if (!m_hWnd)
		{
			DWORD dwError = GetLastError();

			MessageBox(NULL, L"Call to CreateWindowEx failed!", ENGINE_NAME, 0u);

			if (dwError != ERROR_CLASS_ALREADY_EXISTS)
			{
				return HRESULT_FROM_WIN32(dwError);
			}

			return E_FAIL;
		}

		ShowWindow(m_hWnd, nCmdShow);

		return S_OK;
	}
}