//Globals.h
#pragma once

#include <windows.h>
#include <ctffunc.h>

// {AAAAAAAA-BBBB-CCCC-DDDD-111111111111}
// �� ������ GUID�� ���� �����Ͽ� ����ϴ� ���� �����մϴ�. (Visual Studio > ���� > GUID �����)
// KRIME Text Service�� CLSID
extern const CLSID c_clsidKRIME;
// KRIME Profile�� GUID
extern const GUID c_guidProfile;
// �ѿ���ȯ ����Ű�� GUID
extern const GUID c_guidLangChangeKey;

// ���� ����
extern HINSTANCE g_hModule;
extern long g_cRefDll;
