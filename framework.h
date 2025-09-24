//framework.h: 공용 헤더 파일
#pragma once
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

// Target Windows version (필수)
#include <sdkddkver.h>

// 자주 사용하지 않는 API는 제외하여 빌드 속도 향상
#define WIN32_LEAN_AND_MEAN

// Windows 및 COM 필수 헤더
#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <ShlObj.h>

#pragma comment(lib, "Shlwapi.lib")

// TSF 필수 헤더
#include <msctf.h>
#include <ctffunc.h>

// --- 프로젝트 공용 헤더 ---
#include "resource.h"
#include "Log.h"
#include "Globals.h"
#include "Register.h"
#include "ClassFactory.h"
