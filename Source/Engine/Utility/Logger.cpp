#include "Pch.h"
#include "Utility/Logger.h"

#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

namespace esperanza
{
	namespace log
	{
		eVerbosity g_CurrentVerbosity = eVerbosity::All;

		constexpr const size_t MAX_LOG_BUFFER_SIZE = 256u;
		WCHAR g_szLogBuffer[MAX_LOG_BUFFER_SIZE] = { L'\0', };
		std::queue<std::wstring> g_StringQueue;
		BOOL g_bIsStringPrinting = TRUE;
		std::wostringstream g_Os;
		std::thread g_LogThread;
		std::mutex g_QueueMutex;

		void ProcessLog() noexcept;

		void Initialize(eVerbosity verbosity) noexcept
		{
			SetVerbosity(verbosity);
			g_bIsStringPrinting = TRUE;

			g_LogThread = std::thread(ProcessLog);
		}

		void Destroy() noexcept
		{
			while (!g_StringQueue.empty())
			{
			}

			g_bIsStringPrinting = FALSE;
			g_LogThread.join();
		}

		void SetVerbosity(eVerbosity verbosity) noexcept
		{
			g_CurrentVerbosity = verbosity;
		}

		void PrintLog(eVerbosity verbosity, const char* pszFileName, const char* pszFunctionName, uint32_t uLineNumber, const wchar_t* pszMessage) noexcept
		{
			std::lock_guard<std::mutex> lock(g_QueueMutex);

			if (verbosity < g_CurrentVerbosity)
			{
				return;
			}

			std::wstring szLogBuffer;

			g_Os.flush();
			g_Os.clear();

			g_Os << pszFileName << L'/' << pszFunctionName << L"line: " << uLineNumber << L" :\t" << pszMessage << std::endl;
			szLogBuffer.append(g_Os.str());
			g_StringQueue.push(szLogBuffer);
		}

		void PrintLogFormat(eVerbosity verbosity, const char* pszFileName, const char* pszFunctionName, uint32_t uLineNumber, const wchar_t* pszMessage, ...) noexcept
		{
			va_list vl;
			va_start(vl, pszMessage);
			vswprintf(g_szLogBuffer, MAX_LOG_BUFFER_SIZE, pszMessage, vl);
			PrintLog(verbosity, pszFileName, pszFunctionName, uLineNumber, g_szLogBuffer);
			va_end(vl);
		}

		void ProcessLog() noexcept
		{
			while (g_bIsStringPrinting)
			{
				std::lock_guard<std::mutex> lock(g_QueueMutex);

				while (!g_StringQueue.empty())
				{
					OutputDebugString(g_StringQueue.front().c_str());
					g_StringQueue.pop();
				}
			}
		}
	}
}
