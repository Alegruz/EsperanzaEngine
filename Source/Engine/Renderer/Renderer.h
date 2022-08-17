#pragma once

#include "Pch.h"

namespace esperanza
{
	class Renderer final
	{
	public:
		explicit Renderer() noexcept = default;
		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;
		~Renderer() noexcept = default;

		HRESULT Initialize(_In_ HWND hWnd) noexcept;
		void Update(_In_ FLOAT deltaTime) noexcept;
		void Render() noexcept;

	private:

	};
}