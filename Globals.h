//Globals.h
#pragma once

#include <windows.h>
#include <ctffunc.h>

// {AAAAAAAA-BBBB-CCCC-DDDD-111111111111}
// 위 형식의 GUID를 새로 생성하여 사용하는 것을 권장합니다. (Visual Studio > 도구 > GUID 만들기)
// KRIME Text Service의 CLSID
extern const CLSID c_clsidKRIME;
// KRIME Profile의 GUID
extern const GUID c_guidProfile;
// 한영변환 단축키의 GUID
extern const GUID c_guidLangChangeKey;

// 전역 변수
extern HINSTANCE g_hModule;
extern long g_cRefDll;
