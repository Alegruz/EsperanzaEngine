#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wrl.h>

// DirectX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <d3dx12.h>

// C++
#include <memory>

using namespace Microsoft::WRL;

namespace esperanza
{
	constexpr const WCHAR ENGINE_NAME[] = L"Esperanza";
}