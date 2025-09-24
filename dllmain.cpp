#include "framework.h"

// 전역 변수 정의
HINSTANCE g_hModule = NULL;
long g_cRefDll = 0;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    g_hModule = hModule; // 이 작업만 남겨둡니다.
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// 표준 COM DLL 내보내기 함수들 (이하 변경 없음)
STDAPI DllCanUnloadNow(void)
{
    return (g_cRefDll > 0) ? S_FALSE : S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    if (!IsEqualCLSID(rclsid, c_clsidKRIME))
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    CClassFactory* pFactory = new CClassFactory();
    if (pFactory == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

STDAPI DllRegisterServer(void)
{
    return RegisterServer();
}

STDAPI DllUnregisterServer(void)
{
    return UnregisterServer();
}