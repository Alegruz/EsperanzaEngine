#include "Pch.h"

#include "Game/Game.h"
#include "Renderer/Renderer.h"
#include "Window/MainWindow.h"

namespace esperanza
{
	Game::Game(PCWSTR pszGameName) noexcept
		: m_pszGameName(pszGameName)
		, m_pMainWindow(std::make_unique<MainWindow>())
	{
	}

	Game::~Game() noexcept = default;

	HRESULT Game::Initialize(HINSTANCE hInstance, INT nCmdShow) noexcept
	{
		HRESULT hr = S_OK;

		hr = m_pMainWindow->Initialize(hInstance, nCmdShow, m_pszGameName);
		if (FAILED(hr))
		{
			return hr;
		}

		hr = m_pRenderer->Initialize(m_pMainWindow->GetWindowHandle());
		if (FAILED(hr))
		{
			return hr;
		}

		return hr;
	}

	INT Game::Run() noexcept
	{
		MSG msg = { 0 };

		LARGE_INTEGER startingTime;
		LARGE_INTEGER endingTime;
		LARGE_INTEGER elapsedMicroseconds;
		LARGE_INTEGER frequency;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&startingTime);

        while (WM_QUIT != msg.message)
        {
            if (PeekMessage(&msg, NULL, 0u, 0u, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                // Update our time
                QueryPerformanceCounter(&endingTime);
                elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
                elapsedMicroseconds.QuadPart *= 1000000;
                elapsedMicroseconds.QuadPart /= frequency.QuadPart;
                QueryPerformanceFrequency(&frequency);
                QueryPerformanceCounter(&startingTime);

                FLOAT deltaTime = static_cast<FLOAT>(elapsedMicroseconds.QuadPart) / 1000000.0f;

				m_pRenderer->Update(deltaTime);
				m_pRenderer->Render();
            }
        }

        return static_cast<INT>(msg.wParam);
	}
}