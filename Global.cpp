#include "pch.h"
#include <initguid.h> // DEFINE_GUID를 정의로 만들기 위해 가장 먼저 포함해야 합니다.
#include "Global.h"

// --- 전역 변수 정의 ---
HINSTANCE g_hInstDll = NULL;
LONG g_dllRefCount = -1;