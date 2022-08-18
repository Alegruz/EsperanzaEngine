#pragma once

// Windows
#ifndef  UNICODE
#define UNICODE
#endif // ! UNICODE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // ! WIN32_LEAN_AND_MEAN

#include <windows.h>

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include <wrl.h>
#include <comdef.h>

// DirectX
#include <d3d12.h>
#include <dxgi1_6.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <d3dx12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// STL Headers
#include <cassert>
#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#include "Resource.h"

#include "Utility/Logger.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace esperanza
{
	inline constexpr size_t ConvertKbToBytes(size_t kilobytes)
	{
		return kilobytes * 1024;
	}

	inline constexpr size_t ConvertMbToBytes(size_t megabytes)
	{
		return megabytes * 1024 * 1024;
	}

	const std::filesystem::path CONTENTS_PATH(L"Contents");
	const std::filesystem::path SHADERS_PATH(CONTENTS_PATH / L"Shaders");

	constexpr const WCHAR ENGINE_NAME[] = L"Esperanza";
	constexpr const size_t DEFAULT_WIDTH = 1920;
	constexpr const size_t DEFAULT_HEIGHT = 1080;

	constexpr const size_t _64KB = ConvertKbToBytes(64);
	constexpr const size_t _1MB = ConvertMbToBytes(1);
	constexpr const size_t _2MB = ConvertMbToBytes(2);
	constexpr const size_t _4MB = ConvertMbToBytes(4);
	constexpr const size_t _8MB = ConvertMbToBytes(8);
	constexpr const size_t _16MB = ConvertMbToBytes(16);
	constexpr const size_t _32MB = ConvertMbToBytes(32);
	constexpr const size_t _64MB = ConvertMbToBytes(64);
	constexpr const size_t _128MB = ConvertMbToBytes(128);
	constexpr const size_t _256MB = ConvertMbToBytes(256);
}