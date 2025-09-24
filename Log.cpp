// Log.cpp

#include "framework.h"

#ifdef _DEBUG

#include <stdio.h>
#include <cstdarg>

Log& Log::GetInstance()
{
    static Log instance;
    return instance;
}

void Log::Write(const char* file, int line, const char* function, const char* format, ...)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_file.is_open())
    {
        // �ð� ����
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        tm local_tm;
        localtime_s(&local_tm, &time_t_now);
        m_file << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ";

        // �ҽ��ڵ� ��ġ ����
        const char* filename = strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file;
        m_file << "[" << filename << ":" << line << " (" << function << ")] ";

        // ���� ���� ó��
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        m_file << buffer << std::endl;
        m_file.flush();
    }
}

Log::Log()
{
    PWSTR pszPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &pszPath)))
    {
        std::wstring logFilePath = pszPath;
        logFilePath += L"\\KRIME_log.txt";

        m_file.open(logFilePath, std::ios::out | std::ios::trunc);
    }
    if (pszPath)
    {
        CoTaskMemFree(pszPath);
    }

    if (m_file.is_open())
    {
        // ���� ������ �κ� ����
        // LOG_WRITE ��ũ�� ��� ���� ���Ͽ� ���ϴ�.
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        tm local_tm;
        localtime_s(&local_tm, &time_t_now);
        m_file << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ";
        m_file << "==================== Log Start ====================" << std::endl;
        m_file.flush();
        // ���� ������� ���� ����
    }
}

Log::~Log()
{
    if (m_file.is_open())
    {
        // ���� ������ �κ� ����
        // LOG_WRITE ��ũ�� ��� ���� ���Ͽ� ���ϴ�.
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        tm local_tm;
        localtime_s(&local_tm, &time_t_now);
        m_file << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ";
        m_file << "==================== Log End ======================" << std::endl;
        m_file.flush();
        // ���� ������� ���� ����

        m_file.close();
    }
}

#endif // _DEBUG