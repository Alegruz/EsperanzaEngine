#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Globals
constexpr const WCHAR WINDOW_CLASS_NAME[] = L"WinClass";

// Functions
LRESULT CALLBACK WinProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	WNDCLASSEX wcex =
	{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_VREDRAW | CS_HREDRAW	// Redraws the entire window if a movement or size adjustment changes the width/height of the window. 
				| CS_OWNDC					// Allocates a unique device context for each window in the class
				| CS_DBLCLKS,				// Sends a double-click message to the window procedure when the user double - clicks the mouse while the cursor is in a window belonging to the class.
		.lpfnWndProc = WinProc,
		.cbClsExtra = 0,					// extra class info space
		.cbWndExtra = 0,					// extra window info space
		.hInstance = hInstance,				// assign the application instance
		.hIcon = LoadIcon(NULL, IDI_APPLICATION),	// Default application icon
		.hCursor = LoadCursor(NULL, IDC_ARROW),		// Standard arrow
		.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)),
		.lpszMenuName = nullptr,			// the name of the menu to attach
		.lpszClassName = WINDOW_CLASS_NAME,		// the name of the class itself
		.hIconSm = LoadIcon(NULL, IDI_APPLICATION),	// the handle of the small icon
	};

	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(
		NULL,					// extended style
		WINDOW_CLASS_NAME,			// class
		L"Your Basic Window",	// title
		WS_OVERLAPPEDWINDOW		// An overlapped window with the WS_OVERLAPPED, WS_CAPTION, WS_SYSMENU, WS_THICKFRAME, WS_MINIMIZEBOX, and WS_MAXIMIZEBOX styles.
									// WS_OVERLAPPED - An overlapped window, which has a title bar and a border. Same as the WS_TILED style
									// WS_CAPTION - A window that has a title bar (includes the WS_BORDER style).
									// WS_SYSMENU - A window that has a window menu on its title bar.The WS_CAPTION style must also be specified.
									// WS_MINIMIZEBOX - A window that has a Minimize button. Cannot be combined with the WS_EX_CONTEXTHELP style.The WS_SYSMENU style must also be specified.
									// WS_MAXIMIZEBOX - A window that has a Maximize button. Cannot be combined with the WS_EX_CONTEXTHELP style.The WS_SYSMENU style must also be specified.
		| WS_VISIBLE,			// A window that is initially visible.
		0, 0,					// Initial x, y
		1280, 720,				// Initial width, height
		NULL,					// handle to parent
		NULL,					// handle to menu
		hInstance,				// instance of this application
		NULL					// extra creation params
	);

	if (!hWnd)
	{
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);

	MSG msg = {};

	// Main event loop
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Translate any accelerator keys
			TranslateMessage(&msg);
			// Send the message to the window proc
			DispatchMessage(&msg);
		}
	}

	// Exit program
	return 0;
}

LRESULT WinProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg)
	{
	case WM_CREATE:
	{
		// Initialize
		return 0;
	}
	break;
	case WM_PAINT:
	{
		// Validate the window
		hdc = BeginPaint(hWnd, &ps);
		// Paint
		EndPaint(hWnd, &ps);
		return 0;
	}
	break;
	case WM_DESTROY:
	{
		// Kill the application
		PostQuitMessage(0);
		return 0;
	}
	break;
	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
