#pragma once

#include "Window/BaseWindow.h"

namespace esperanza
{
	class MainWindow final : public BaseWindow<MainWindow>
	{
	public:
		explicit constexpr MainWindow() noexcept = default;
		MainWindow(const MainWindow& other) = delete;
		MainWindow(MainWindow&& other) = delete;
		MainWindow& operator=(const MainWindow& other) = delete;
		MainWindow& operator=(MainWindow&& other) = delete;
		constexpr ~MainWindow() noexcept = default;

		HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) noexcept override;
		constexpr PCWSTR GetWindowClassName() const noexcept override;
		LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) noexcept override;
	};
}