#pragma once

#include "Pch.h"

namespace esperanza
{
	class MainWindow;
	class Renderer;

	class Game final
	{
	public:
		Game() = delete;
		explicit Game(_In_ PCWSTR pszGameName) noexcept;
		Game(const Game& other) = delete;
		Game(Game&& other) = delete;
		Game& operator=(const Game& other) = delete;
		Game& operator=(Game&& other) = delete;
		~Game() noexcept;

		HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow) noexcept;
		void Destroy() noexcept;
		INT Run() noexcept;

	private:
		static DWORD WINAPI run(LPVOID lpParameter) noexcept;

	private:
		PCWSTR m_pszGameName;
		std::unique_ptr<MainWindow> m_pMainWindow;
		std::unique_ptr<Renderer> m_pRenderer;
		Log m_Logger;

		HANDLE m_hMainThread;
		DWORD m_dwThreadId;
		HANDLE m_hTaskWakeUpEvent;
		BOOL m_bIsRunning;
	};
}