#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

// ����� ����(_DEBUG�� ���ǵ�)�� ���� �α� ����� Ȱ��ȭ
#ifdef _DEBUG

class Log
{
public:
    static Log& GetInstance();
    void Write(const char* file, int line, const char* function, const char* format, ...);

    Log(const Log&) = delete;
    void operator=(const Log&) = delete;

private:
    Log();
    ~Log();

    std::ofstream m_file;
    std::mutex m_mutex;
};

// �α� �ۼ��� ���� ��ũ��: ����, ��, �Լ� ������ �ڵ����� �Ѱ��ݴϴ�.
#define LOG_WRITE(...) Log::GetInstance().Write(__FILE__, __LINE__, __func__, __VA_ARGS__)

#else // ������ ������ ���

// ��� �α� ȣ���� ��������� ��ũ��
#define LOG_WRITE(...)

#endif // _DEBUG