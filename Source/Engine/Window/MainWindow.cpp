#include "Pch.h"
#include "Window/MainWindow.h"

namespace esperanza
{
	HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) noexcept
	{
		RAWINPUTDEVICE rid =
		{
			.usUsagePage = 0x01,	// MOUSE	
			.usUsage = 0X02,
			.dwFlags = 0u,			// Default flags
			.hwndTarget = NULL
		};
		
		if (!RegisterRawInputDevices(&rid, 1u, sizeof(rid)))
		{
			return E_FAIL;
		}

		return initialize(
			hInstance, 
			nCmdShow, 
			pszWindowName, 
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
			CW_USEDEFAULT, CW_USEDEFAULT, 
			1280, 720
		);
	}

	constexpr PCWSTR MainWindow::GetWindowClassName() const noexcept
	{
		return L"EngineWindow";
	}

	LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (uMsg)
		{
		case WM_CLOSE:
		{
			if (MessageBox(m_hWnd, L"Really Quit?", ENGINE_NAME, MB_OKCANCEL) == IDOK)
			{
				HMENU hMenu = GetMenu(m_hWnd);
				if (hMenu)
				{
					DestroyMenu(hMenu);
				}
				DestroyWindow(m_hWnd);
				UnregisterClass(GetWindowClassName(), m_hInstance);
			}
		}
		break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
		case WM_INPUT:
		{
			UINT uDataSize = 0u;

			GetRawInputData(
				reinterpret_cast<HRAWINPUT>(lParam),
				RID_INPUT,
				nullptr,
				&uDataSize,
				sizeof(RAWINPUTHEADER)
			);
			if (uDataSize > 0)
			{
				std::unique_ptr<BYTE[]> rawData = std::make_unique<BYTE[]>(uDataSize);
				if (GetRawInputData(
					reinterpret_cast<HRAWINPUT>(lParam),
					RID_INPUT,
					rawData.get(),
					&uDataSize,
					sizeof(RAWINPUTHEADER)
				))
				{
					RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawData.get());
					if (raw->header.dwType == RIM_TYPEMOUSE)
					{
						RECT rc;
						RECT rc2;
						POINT p1;
						POINT p2;

						GetWindowRect(m_hWnd, &rc2);
						GetClientRect(m_hWnd, &rc);
						p1.x = rc.left;
						p1.y = rc.top;
						p2.x = rc.right;
						p2.y = rc.bottom;

						ClientToScreen(m_hWnd, &p1);
						ClientToScreen(m_hWnd, &p2);

						rc.left = p1.x;
						rc.top = rc2.top;
						rc.right = p2.x;
						rc.bottom = p2.y;

						ClipCursor(&rc);
					}
				}
			}
		}
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(m_hWnd, &ps);
			EndPaint(m_hWnd, &ps);
		}
		break;
		default:
			return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}
}