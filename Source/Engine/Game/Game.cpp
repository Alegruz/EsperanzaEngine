#include "Pch.h"

#include "Game/Game.h"
#include "Input/KeyboardInput.h"
#include "Renderer/Renderer.h"
#include "Window/MainWindow.h"

namespace esperanza
{
	Game::Game(PCWSTR pszGameName) noexcept
		: m_pszGameName(pszGameName)
		, m_pMainWindow(std::make_unique<MainWindow>())
		, m_pRenderer(std::make_unique<Renderer>())
		, m_Logger()
		, m_hMainThread()
		, m_dwThreadId()
		, m_hTaskWakeUpEvent()
		, m_bIsRunning(FALSE)
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

		hr = m_pRenderer->Initialize(*m_pMainWindow);
		if (FAILED(hr))
		{
			return hr;
		}

		Log::eVerbosity verbosity = Log::eVerbosity::All;

#ifdef NDEBUG
		verbosity = Log::eVerbosity::Error;
#endif

		g_Log.Initialize(verbosity);
		m_Logger.Initialize(verbosity);

		return hr;
	}

	void Game::Destroy() noexcept
	{
		m_pRenderer->Destroy();

		m_Logger.Destroy();
		g_Log.Destroy();
	}

	INT Game::Run() noexcept
	{
		MSG msg = { 0 };

		// Game Programming Gems 1. Chapter 1.12: Linear Programming Model for Windows-based Games. 2001.
		m_bIsRunning = TRUE;
		m_hTaskWakeUpEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!m_hTaskWakeUpEvent)
		{
			DWORD dwError = GetLastError();

			LOGEF(m_Logger, L"CreateEvent Main Thread Task Wake Up Event failed with error code %u", dwError);

			if (dwError != ERROR_CLASS_ALREADY_EXISTS)
			{
				return HRESULT_FROM_WIN32(dwError);
			}

			return E_FAIL;
		}

		SetEvent(m_hTaskWakeUpEvent);
		m_pMainWindow->SetCallbackToWindowMessage(
			WM_ACTIVATE,
			[](WPARAM wParam, LPARAM lParam, ULONGLONG ullParam)
			{
				UNREFERENCED_PARAMETER(lParam);
				BOOL bIsActive = LOWORD(wParam) != WA_ACTIVE;
				HANDLE hEvent = reinterpret_cast<HANDLE>(ullParam);

				if (bIsActive)
				{
					ResetEvent(hEvent);
				}
				else
				{
					SetEvent(hEvent);
				}
			},
			reinterpret_cast<ULONGLONG>(m_hTaskWakeUpEvent)
				);

		m_hMainThread = CreateThread(NULL, 0ull, &Game::run, this, 0u, &m_dwThreadId);
		if (!m_hMainThread)
		{
			DWORD dwError = GetLastError();

			LOGEF(m_Logger, L"CreateThread Main Thread failed with error code %u", dwError);
			return -1;
		}

		// Window Message Loop
        while (WM_QUIT != msg.message)
        {
            if (PeekMessage(&msg, NULL, 0u, 0u, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
		m_bIsRunning = FALSE;
		SetEvent(m_hTaskWakeUpEvent);

		WaitForSingleObject(m_hMainThread, INFINITE);

		CloseHandle(m_hMainThread);
		CloseHandle(m_hTaskWakeUpEvent);

        return static_cast<INT>(msg.wParam);
	}

	DWORD __stdcall Game::run(LPVOID lpParameter) noexcept
	{
		LARGE_INTEGER startingTime;
		LARGE_INTEGER endingTime;
		LARGE_INTEGER elapsedMicroseconds;
		LARGE_INTEGER frequency;

		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&startingTime);

		while (reinterpret_cast<Game*>(lpParameter)->m_bIsRunning)
		{
			// Update our time
			QueryPerformanceCounter(&endingTime);
			elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
			elapsedMicroseconds.QuadPart *= 1000000;
			elapsedMicroseconds.QuadPart /= frequency.QuadPart;
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&startingTime);

			FLOAT deltaTime = static_cast<FLOAT>(elapsedMicroseconds.QuadPart) / 1000000.0f;
			//LOGIF(reinterpret_cast<Game*>(lpParameter)->m_Logger, L"Elapsed Time: %f", deltaTime);

			// Handle Input
			reinterpret_cast<Game*>(lpParameter)->m_pRenderer->Update(deltaTime);
			reinterpret_cast<Game*>(lpParameter)->m_pRenderer->Render();
			// Game Programming Gems 1. Chapter 1.12: Linear Programming Model for Windows-based Games. 2001.
			WaitForSingleObject(reinterpret_cast<Game*>(lpParameter)->m_hTaskWakeUpEvent, INFINITE);
		}

		ExitThread(0);
	}
}