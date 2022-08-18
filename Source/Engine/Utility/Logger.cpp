#include "Pch.h"
#include "Utility/Logger.h"

namespace esperanza
{
	Log g_Log;

	Log::Log() noexcept
		: m_CurrentVerbosity(eVerbosity::All)
		, m_szLogBuffer{ L'\0', }
		, m_StringQueue()
		, m_bIsStringPrinting(TRUE)
		, m_Os()
		, m_LogThread()
		, m_QueueMutex()
	{
	}

	void Log::Initialize(eVerbosity verbosity) noexcept
	{
		SetVerbosity(verbosity);
		m_bIsStringPrinting = TRUE;

		m_LogThread = std::thread(processLog, &m_bIsStringPrinting, &m_QueueMutex, &m_StringQueue);
	}

	void Log::Destroy() noexcept
	{
		while (!m_StringQueue.empty())
		{
		}

		m_bIsStringPrinting = FALSE;
		m_LogThread.join();
	}

	void Log::SetVerbosity(eVerbosity verbosity) noexcept
	{
		m_CurrentVerbosity = verbosity;
	}

	void Log::PrintLog(eVerbosity verbosity, const char* pszFileName, const char* pszFunctionName, uint32_t uLineNumber, const wchar_t* pszMessage) noexcept
	{
		std::lock_guard<std::mutex> lock(m_QueueMutex);

		if (verbosity < m_CurrentVerbosity)
		{
			return;
		}

		std::wstring szLogBuffer;

		m_Os.flush();
		m_Os.clear();

		m_Os << pszFileName << L'/' << pszFunctionName << L"line: " << uLineNumber << L" :\t" << pszMessage << std::endl;
		szLogBuffer.append(m_Os.str());
		m_StringQueue.push(szLogBuffer);
	}

	void Log::PrintLogFormat(eVerbosity verbosity, const char* pszFileName, const char* pszFunctionName, uint32_t uLineNumber, const wchar_t* pszMessage, ...) noexcept
	{
		va_list vl;
		va_start(vl, pszMessage);
		vswprintf(m_szLogBuffer, MAX_LOG_BUFFER_SIZE, pszMessage, vl);
		PrintLog(verbosity, pszFileName, pszFunctionName, uLineNumber, m_szLogBuffer);
		va_end(vl);
	}

	void Log::processLog(const BOOL* bIsStringPrinting, std::mutex* queueMutex, std::queue<std::wstring>* stringQueue) noexcept
	{
		while (*bIsStringPrinting)
		{
			std::lock_guard<std::mutex> lock(*queueMutex);

			while (!stringQueue->empty())
			{
				OutputDebugString(stringQueue->front().c_str());
				stringQueue->pop();
			}
		}
	}
}
