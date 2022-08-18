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
			DWORD dwError = GetLastError();

			GLOGEF(L"Registering Raw Input Device failed with error code %u", dwError);

			if (dwError != ERROR_CLASS_ALREADY_EXISTS)
			{
				return HRESULT_FROM_WIN32(dwError);
			}

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

#if _DEBUG
#define PRINT_HANDLE_MESSAGE_LOG (0)
#endif

	LRESULT MainWindow::HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) noexcept
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:
		{
			// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
			// Game Coding Complete. Mike McShaffry. 2012.
			// Sent to both the window being activated and the window being deactivated.

			m_bIsActive = LOWORD(wParam) == WA_ACTIVE;

			// Pause or resume audio
			if (m_bIsActive)
			{
				// Resume
#if PRINT_HANDLE_MESSAGE_LOG
				GLOGIF(L"WM_ACTIVATE - Activated. Minimized State: %s", HIWORD(wParam) ? L"Minimized" : L"");
#endif

				// Set the mode if it has changed
			}
			else
			{
				// Pause
#if PRINT_HANDLE_MESSAGE_LOG
				GLOGIF(L"WM_ACTIVATE - Inactivated. Minimized State: %s", HIWORD(wParam) ? L"Minimized" : L"");
#endif
			}

			// It's also a good idea is to save your current backbuffer in a regular offscreen surface. 
			// Why bother? It turns out that when a player hits Ctrl-AltDel, video surfaces can be lost. 
			// This is an excellent way to force your game to suffer a surface loss condition.
			// If you have a copy of your backbuffer in off - screen memory, 
			// you can use it to paint while your application is inactive.
			// Just because your game is inactive doesn't mean that it doesn't get paint messages.
			// You might drag another window over your game.
		}
		break;
		case WM_CLOSE:
		{
			// Sent as a signal that a window or an application should terminate.
#if PRINT_HANDLE_MESSAGE_LOG
			GLOGI(L"WM_CLOSE");
#endif
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
#if PRINT_HANDLE_MESSAGE_LOG
			GLOGI(L"WM_DESTROY");
#endif
			PostQuitMessage(0);
		}
		break;
		case WM_DISPLAYCHANGE:
		{
			// Game Coding Complete. Mike McShaffry. 2012.
			// The WM_DISPLAYCHANGE message is sent to all windows when the display resolution has changed.
			m_rcDesktop = RECT(0, 0, LOWORD(lParam), HIWORD(lParam));
			m_uColorDepth = static_cast<UINT>(wParam);

#if PRINT_HANDLE_MESSAGE_LOG
			GLOGIF(L"WM_DISPLAYCHANGE - SIZE: (%d, %d) DEPTH: %u", m_rcDesktop.right, m_rcDesktop.bottom, m_uColorDepth);
#endif
		}
		break;
		case WM_GETMINMAXINFO:
		{
			// Game Coding Complete. Mike McShaffry. 2012.
			// Sent to a window when the size or position of the window is about to change. 
			// An application can use this message to override the window's default maximized size and position, 
			// or its default minimum or maximum tracking size.

			// Effectively keeps the window at exactly it's current size!
#if PRINT_HANDLE_MESSAGE_LOG
			GLOGI(L"WM_GETMINMAXINFO");
#endif

			LPMINMAXINFO lpMinMaxInfo = reinterpret_cast<LPMINMAXINFO>(lParam);

			lpMinMaxInfo->ptMinTrackSize.x = lpMinMaxInfo->ptMaxTrackSize.y = m_rcWindow.right - m_rcWindow.left;
			lpMinMaxInfo->ptMinTrackSize.y = lpMinMaxInfo->ptMaxTrackSize.y = m_rcWindow.bottom - m_rcWindow.top;
		}
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
		case WM_KEYDOWN:
		{
			// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
			// Get virtual key code and data bits
			UINT uVirtualCode = static_cast<UINT>(wParam);
			UINT uKeyState = static_cast<UINT>(lParam);

			// Switch on the virtual key code to be clean

#if PRINT_HANDLE_MESSAGE_LOG
			WCHAR szKeyName[16];

			GetKeyNameText(static_cast<LONG>(lParam), szKeyName, 16);

			GLOGIF(L"WM_KEYDOWN (%u, %u): %s", uVirtualCode, uKeyState, szKeyName);
#else
			UNREFERENCED_PARAMETER(uVirtualCode);
			UNREFERENCED_PARAMETER(uKeyState);
#endif
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
			// Posted when the user double-clicks the left mouse button while the cursor is in the client area of a window. 
			// If the mouse is not captured, the message is posted to the window beneath the cursor. 
			// Otherwise, the message is posted to the window that has captured the mouse.
			SHORT x = static_cast<SHORT>(LOWORD(lParam));
			SHORT y = static_cast<SHORT>(HIWORD(lParam));

#if PRINT_HANDLE_MESSAGE_LOG
			GLOGIF(L"WM_LBUTTONDBLCLK (%d, %d)", x, y);
#else
			UNREFERENCED_PARAMETER(x);
			UNREFERENCED_PARAMETER(y);
#endif
		}
		break;
		case WM_MOVE:
		{
			// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
			// Game Coding Complete. Mike McShaffry. 2012.
			// Sent after a window has been moved.
			WORD x = LOWORD(lParam);
			WORD y = HIWORD(lParam);

#if PRINT_HANDLE_MESSAGE_LOG
			GLOGIF(L"WM_MOVE (%u, %u)", x, y);
#else
			UNREFERENCED_PARAMETER(x);
			UNREFERENCED_PARAMETER(y);
#endif

			//if (m_bIsWindowed)
			//{
			//	GetClientRect(m_hWnd, &m_rcWindow);
			//	ClientToScreen(m_hWnd, reinterpret_cast<LPPOINT>(&m_rcWindow));
			//}
			//else
			//{
			//	SetRect(&m_rcWindow, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
			//}
		}
		break;
		case WM_MOVING:
		{
			// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
			// Sent to a window that the user is moving. 
			// By processing this message, an application can monitor the position of the drag rectangle and, if needed, change its position.
			RECT* pRect = reinterpret_cast<RECT*>(lParam);

#if PRINT_HANDLE_MESSAGE_LOG
			GLOGIF(L"WM_MOVING (%d, %d), (%d, %d)", pRect->left, pRect->top, pRect->right, pRect->bottom);
#else
			UNREFERENCED_PARAMETER(pRect);
#endif
		}
		break;
		case WM_MOUSEMOVE:
		{
			// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
			// Posted to a window when the cursor moves. 
			// If the mouse is not captured, the message is posted to the window that contains the cursor. 
			// Otherwise, the message is posted to the window that has captured the mouse.
			SHORT x = static_cast<SHORT>(LOWORD(lParam));
			SHORT y = static_cast<SHORT>(HIWORD(lParam));

			UINT uButtons = static_cast<UINT>(wParam);

			if (uButtons & MK_LBUTTON)
			{
#if PRINT_HANDLE_MESSAGE_LOG
				GLOGIF(L"WM_MOUSEMOVE (%d, %d), LBUTTON", x, y);
#endif
			}
			
			if (uButtons & MK_MBUTTON)
			{
#if PRINT_HANDLE_MESSAGE_LOG
				GLOGIF(L"WM_MOUSEMOVE (%d, %d), MBUTTON", x, y);
#endif
			}

			if (uButtons & MK_RBUTTON)
			{
#if PRINT_HANDLE_MESSAGE_LOG
				GLOGIF(L"WM_MOUSEMOVE (%d, %d), RBUTTON", x, y);
#endif
			}

#if !PRINT_HANDLE_MESSAGE_LOG
			UNREFERENCED_PARAMETER(x);
			UNREFERENCED_PARAMETER(y);
#endif
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(m_hWnd, &ps);
			EndPaint(m_hWnd, &ps);
		}
		break;
		case WM_POWERBROADCAST:
		{
			// Game Coding Complete. Mike McShaffry. 2012.
			// Notifies applications that a power-management event has occurred.

			// Don't allow the game to go into sleep mode
			if (wParam == PBT_APMQUERYSTANDBY)
			{
				return BROADCAST_QUERY_DENY;
			}
			else if (wParam == PBT_APMBATTERYLOW)
			{
				// Save and Exit game
			}
		}
		break;
		case WM_SIZE:
		{
			// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
			// Sent to a window after its size has changed.

			WORD wWidth = LOWORD(lParam);
			WORD wHeight = HIWORD(lParam);

#if PRINT_HANDLE_MESSAGE_LOG
			GLOGIF(L"WM_SIZE (%u x %u)", wWidth, wHeight);
#else
			UNREFERENCED_PARAMETER(wWidth);
			UNREFERENCED_PARAMETER(wHeight);
#endif
		}
		break;
		case WM_SIZING:
		{
			// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
			// Sent to a window that the user is resizing. By processing this message, an application can monitor the size and position of the drag rectangle and, if needed, change its size or position.
			RECT* pRect = reinterpret_cast<RECT*>(lParam);
#if PRINT_HANDLE_MESSAGE_LOG
			const WCHAR* szEdge = nullptr;
			switch (wParam)
			{
			case WMSZ_BOTTOM:
			{
				szEdge = L"BOTTOM";
			}
			break;
			case WMSZ_BOTTOMLEFT:
			{
				szEdge = L"BOTTOMLEFT";
			}
			break;
			case WMSZ_BOTTOMRIGHT:
			{
				szEdge = L"BOTTOMRIGHT";
			}
			break;
			case WMSZ_LEFT:
			{
				szEdge = L"LEFT";
			}
			break;
			case WMSZ_RIGHT:
			{
				szEdge = L"RIGHT";
			}
			break;
			case WMSZ_TOP:
			{
				szEdge = L"TOP";
			}
			break;
			case WMSZ_TOPLEFT:
			{
				szEdge = L"TOPLEFT";
			}
			break;
			case WMSZ_TOPRIGHT:
			{
				szEdge = L"TOPRIGHT";
			}
			break;
			default:
				break;
			}

			GLOGIF(L"WM_SIZING Edge: %s (%d, %d), (%d, %d)", szEdge, pRect->left, pRect->top, pRect->right, pRect->bottom);
#else
			UNREFERENCED_PARAMETER(pRect);
#endif
		}
		break;
		default:
			return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		}

		if (m_Callbacks.contains(uMsg))
		{
			m_Callbacks[uMsg].first(wParam, lParam, m_Callbacks[uMsg].second);
		}

		return 0;
	}
}