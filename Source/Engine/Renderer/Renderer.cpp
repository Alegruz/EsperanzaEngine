#include "Pch.h"
#include "Renderer/Renderer.h"

namespace esperanza
{
	HRESULT Renderer::Initialize(HWND hWnd) noexcept
	{
		UNREFERENCED_PARAMETER(hWnd);

		HRESULT hr = S_OK;

		return hr;
	}

	void Renderer::Update(FLOAT deltaTime) noexcept
	{
		UNREFERENCED_PARAMETER(deltaTime);
	}

	void Renderer::Render() noexcept
	{
	}
}