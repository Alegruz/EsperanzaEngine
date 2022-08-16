#define WIN32_LEAN_AND_MEAN
#include <windows.h>

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	// Call message box API with NULL for parent window handle
	MessageBox(NULL, L"There can be only one!!!", L"My First Windows Program", MB_OK | MB_ICONEXCLAMATION);

	// Exit program
	return 0;
}