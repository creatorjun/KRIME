#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

// 디버그 빌드(_DEBUG가 정의됨)일 때만 로그 기능을 활성화
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

// 로그 작성을 위한 매크로: 파일, 줄, 함수 정보를 자동으로 넘겨줍니다.
#define LOG_WRITE(...) Log::GetInstance().Write(__FILE__, __LINE__, __func__, __VA_ARGS__)

#else // 릴리스 빌드일 경우

// 모든 로그 호출을 비워버리는 매크로
#define LOG_WRITE(...)

#endif // _DEBUG