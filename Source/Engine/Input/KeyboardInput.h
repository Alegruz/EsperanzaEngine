#pragma once

#include "Pch.h"

namespace esperanza
{
	class KeyboardInput final
	{
	public:
		static constexpr BOOL IsKeyDown(BYTE key) noexcept;
		static constexpr BOOL IsKeyUp(BYTE key) noexcept;

	public:
		explicit constexpr KeyboardInput() noexcept = default;
		KeyboardInput(const KeyboardInput& other) = delete;
		KeyboardInput(KeyboardInput&& other) = delete;
		KeyboardInput& operator=(const KeyboardInput& other) = delete;
		KeyboardInput& operator=(KeyboardInput&& other) = delete;
		~KeyboardInput() noexcept = default;
	};

	constexpr BOOL KeyboardInput::IsKeyDown(BYTE key) noexcept
	{
		// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
		return (GetAsyncKeyState(key) & 0x8000) ? TRUE : FALSE;
	}

	constexpr BOOL KeyboardInput::IsKeyUp(BYTE key) noexcept
	{
		// Tricks of the Windows Game Programming Gurus, 2nd Ed. André LaMothe. 2003.
		return (GetAsyncKeyState(key) & 0x8000) ? FALSE : TRUE;
	}
}