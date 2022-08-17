#include "Game/Game.h"

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	std::unique_ptr<esperanza::Game> pGame = std::make_unique<esperanza::Game>(L"Game");

	if (FAILED(pGame->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}

	// Exit program
	return pGame->Run();
}