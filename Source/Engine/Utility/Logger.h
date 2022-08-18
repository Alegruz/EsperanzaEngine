#pragma once

#include <cstdarg>
#include <sstream>
#include <string>
#include <thread>

namespace esperanza
{
	class Log;

	extern Log g_Log;

	class Log final
	{
	public:
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

	public:
		explicit Log() noexcept;
		Log(const Log& other) = delete;
		Log(Log&& other) = delete;
		Log& operator=(const Log& other) = delete;
		Log& operator=(Log&& other) = delete;
		~Log() noexcept = default;

		void Initialize(_In_ eVerbosity verbosity) noexcept;
		void Destroy() noexcept;

		void SetVerbosity(_In_ eVerbosity verbosity) noexcept;
		void PrintLog(_In_ eVerbosity verbosity, _In_ const char* pszFileName, _In_ const char* pszFunctionName, _In_ uint32_t uLineNumber, _In_ const wchar_t* pszMessage) noexcept;
		void PrintLogFormat(_In_ eVerbosity verbosity, _In_ const char* pszFileName, _In_ const char* pszFunctionName, _In_ uint32_t uLineNumber, _In_ const wchar_t* pszMessage, ...) noexcept;

	private:
		static void processLog(const BOOL* bIsStringPrinting, std::mutex* queueMutex, std::queue<std::wstring>* stringQueue) noexcept;
	private:
		static constexpr const size_t MAX_LOG_BUFFER_SIZE = 256u;

	private:
		eVerbosity m_CurrentVerbosity;
		WCHAR m_szLogBuffer[MAX_LOG_BUFFER_SIZE];
		std::queue<std::wstring> m_StringQueue;
		BOOL m_bIsStringPrinting;
		std::wostringstream m_Os;
		std::thread m_LogThread;
		std::mutex m_QueueMutex;
	};

#define GLOGF(verbosity, message, ...) (g_Log.PrintLogFormat(verbosity, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOG(verbosity, message, ...) (g_Log.PrintLog(verbosity, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGAF(message, ...) (g_Log.PrintLogFormat(esperanza::Log::eVerbosity::All, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGA(message, ...) (g_Log.PrintLog(esperanza::Log::eVerbosity::All, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGVF(message, ...) (g_Log.PrintLogFormat(esperanza::Log::eVerbosity::Verbose, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGV(message, ...) (g_Log.PrintLog(esperanza::Log::eVerbosity::Verbose, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGIF(message, ...) (g_Log.PrintLogFormat(esperanza::Log::eVerbosity::Info, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGI(message, ...) (g_Log.PrintLog(esperanza::Log::eVerbosity::Info, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGWF(message, ...) (g_Log.PrintLogFormat(esperanza::Log::eVerbosity::Warn, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGW(message, ...) (g_Log.PrintLog(esperanza::Log::eVerbosity::Warn, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGEF(message, ...) (g_Log.PrintLogFormat(esperanza::Log::eVerbosity::Error, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGE(message, ...) (g_Log.PrintLog(esperanza::Log::eVerbosity::Error, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGASF(message, ...) (g_Log.PrintLogFormat(esperanza::Log::eVerbosity::Assert, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define GLOGAS(message, ...) (g_Log.PrintLog(esperanza::Log::eVerbosity::Assert, __FILE__, __func__, __LINE__, message, __VA_ARGS__));

#define LOGF(logger, verbosity, message, ...) (logger.PrintLogFormat(verbosity, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOG(logger, verbosity, message, ...) (logger.PrintLog(verbosity, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGAF(logger, message, ...) (logger.PrintLogFormat(esperanza::Log::eVerbosity::All, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGA(logger, message, ...) (logger.PrintLog(esperanza::Log::eVerbosity::All, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGVF(logger, message, ...) (logger.PrintLogFormat(esperanza::Log::eVerbosity::Verbose, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGV(logger, message, ...) (logger.PrintLog(esperanza::Log::eVerbosity::Verbose, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGIF(logger, message, ...) (logger.PrintLogFormat(esperanza::Log::eVerbosity::Info, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGI(logger, message, ...) (logger.PrintLog(esperanza::Log::eVerbosity::Info, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGWF(logger, message, ...) (logger.PrintLogFormat(esperanza::Log::eVerbosity::Warn, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGW(logger, message, ...) (logger.PrintLog(esperanza::Log::eVerbosity::Warn, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGEF(logger, message, ...) (logger.PrintLogFormat(esperanza::Log::eVerbosity::Error, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGE(logger, message, ...) (logger.PrintLog(esperanza::Log::eVerbosity::Error, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGASF(logger, message, ...) (logger.PrintLogFormat(esperanza::Log::eVerbosity::Assert, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
#define LOGAS(logger, message, ...) (logger.PrintLog(esperanza::Log::eVerbosity::Assert, __FILE__, __func__, __LINE__, message, __VA_ARGS__));
}