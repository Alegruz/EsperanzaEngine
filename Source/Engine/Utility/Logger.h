#pragma once

#include <cstdarg>
#include <cstdint>

namespace esperanza
{
	namespace log
	{
		enum class eVerbosity : uint8_t
		{
			All,
			Verbose,
			Debug,
			Info,
			Warn,
			Error,
			Assert,
			COUNT,
		};

		void Initialize(_In_ eVerbosity verbosity) noexcept;
		void Destroy() noexcept;

		void SetVerbosity(_In_ eVerbosity verbosity) noexcept;
		void PrintLog(_In_ eVerbosity verbosity, _In_ const char* pszFileName, _In_ const char* pszFunctionName, _In_ uint32_t uLineNumber, _In_ const wchar_t* pszMessage) noexcept;
		void PrintLogFormat(_In_ eVerbosity verbosity, _In_ const char* pszFileName, _In_ const char* pszFunctionName, _In_ uint32_t uLineNumber, _In_ const wchar_t* pszMessage, ...) noexcept;

#define LOGF(verbosity, message, ...) (PrintLogFormat(verbosity, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOG(verbosity, message, ...) (PrintLog(verbosity, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
	};
}