// Register.cpp - 수정된 버전
#include "framework.h"
#include "Register.h"
#include <msctf.h>

#define TEXT_SERVICE_NAME L"KRIME Text Service"
#define TEXT_SERVICE_PROFILE_NAME L"KRIME 한글 입력기"

// --- COM 서버 등록 (기존 방식 유지) ---
static BOOL RegisterCOMServer()
{
    LOG_WRITE("RegisterCOMServer started.");

    WCHAR szClsid[MAX_PATH];
    if (StringFromGUID2(c_clsidKRIME, szClsid, ARRAYSIZE(szClsid)) == 0) return FALSE;

    WCHAR szModulePath[MAX_PATH];
    if (GetModuleFileNameW(g_hModule, szModulePath, ARRAYSIZE(szModulePath)) == 0) return FALSE;

    WCHAR szKeyPath[MAX_PATH];

    // 1. CLSID 등록
    StringCchPrintfW(szKeyPath, MAX_PATH, L"CLSID\\%s", szClsid);

    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return FALSE;

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)TEXT_SERVICE_NAME, (DWORD)((wcslen(TEXT_SERVICE_NAME) + 1) * sizeof(WCHAR)));
    RegCloseKey(hKey);

    // 2. InprocServer32 등록
    StringCchPrintfW(szKeyPath, MAX_PATH, L"CLSID\\%s\\InprocServer32", szClsid);
    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return FALSE;

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szModulePath, (DWORD)((wcslen(szModulePath) + 1) * sizeof(WCHAR)));

    LPCWSTR threadingModel = L"Apartment";
    RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, (BYTE*)threadingModel, (DWORD)((wcslen(threadingModel) + 1) * sizeof(WCHAR)));
    RegCloseKey(hKey);

    LOG_WRITE("RegisterCOMServer successful.");
    return TRUE;
}

// --- TSF 등록 (올바른 방식) ---
static BOOL RegisterTSF()
{
    LOG_WRITE("RegisterTSF started.");

    HRESULT hr;

    // 1. ITfInputProcessorProfiles 생성
    ITfInputProcessorProfiles* pInputProcessorProfiles = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles,
        (void**)&pInputProcessorProfiles);

    if (FAILED(hr))
    {
        LOG_WRITE("Failed to create ITfInputProcessorProfiles. HRESULT = 0x%08X", hr);
        return FALSE;
    }

    // 2. 텍스트 서비스 등록
    hr = pInputProcessorProfiles->Register(c_clsidKRIME);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to register text service. HRESULT = 0x%08X", hr);
        pInputProcessorProfiles->Release();
        return FALSE;
    }

    // 3. 언어 프로필 추가 (한국어 - 0x0412)
    WCHAR szModulePath[MAX_PATH];
    GetModuleFileNameW(g_hModule, szModulePath, ARRAYSIZE(szModulePath));

    hr = pInputProcessorProfiles->AddLanguageProfile(c_clsidKRIME,
        0x0412,  // 한국어 LANGID
        c_guidProfile,
        TEXT_SERVICE_PROFILE_NAME,
        static_cast<ULONG>(wcslen(TEXT_SERVICE_PROFILE_NAME)),
        szModulePath,
        static_cast<ULONG>(wcslen(szModulePath)),
        IDI_MAIN);

    if (FAILED(hr))
    {
        LOG_WRITE("Failed to add language profile. HRESULT = 0x%08X", hr);
        pInputProcessorProfiles->Release();
        return FALSE;
    }

    pInputProcessorProfiles->Release();

    // 4. ITfCategoryMgr로 카테고리 등록
    ITfCategoryMgr* pCategoryMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_CategoryMgr,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfCategoryMgr,
        (void**)&pCategoryMgr);

    if (FAILED(hr))
    {
        LOG_WRITE("Failed to create ITfCategoryMgr. HRESULT = 0x%08X", hr);
        return FALSE;
    }

    // 키보드 TIP 카테고리 등록
    hr = pCategoryMgr->RegisterCategory(c_clsidKRIME, GUID_TFCAT_TIP_KEYBOARD, c_clsidKRIME);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to register keyboard category. HRESULT = 0x%08X", hr);
    }

    // UI 요소 지원 카테고리 등록
    hr = pCategoryMgr->RegisterCategory(c_clsidKRIME, GUID_TFCAT_TIPCAP_UIELEMENTENABLED, c_clsidKRIME);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to register UI element category. HRESULT = 0x%08X", hr);
    }

    // Windows 앱 호환성 카테고리 등록 (Windows 8 이상)
    hr = pCategoryMgr->RegisterCategory(c_clsidKRIME, GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, c_clsidKRIME);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to register immersive support category. HRESULT = 0x%08X", hr);
    }

    pCategoryMgr->Release();

    LOG_WRITE("RegisterTSF successful.");
    return TRUE;
}

// --- 메인 등록 함수 ---
HRESULT RegisterServer()
{
    LOG_WRITE("RegisterServer started.");

    // COM 초기화
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to initialize COM. HRESULT = 0x%08X", hr);
        return hr;
    }

    BOOL success = TRUE;

    // 1. COM 서버 등록
    if (!RegisterCOMServer())
    {
        LOG_WRITE("RegisterCOMServer failed.");
        success = FALSE;
    }

    // 2. TSF 등록
    if (success && !RegisterTSF())
    {
        LOG_WRITE("RegisterTSF failed.");
        success = FALSE;
    }

    CoUninitialize();

    LOG_WRITE("RegisterServer %s.", success ? "successful" : "failed");
    return success ? S_OK : E_FAIL;
}

// --- 등록 해제 ---
HRESULT UnregisterServer()
{
    LOG_WRITE("UnregisterServer started.");

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return hr;

    BOOL success = TRUE;

    // 1. TSF 등록 해제
    ITfInputProcessorProfiles* pInputProcessorProfiles = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles,
        (void**)&pInputProcessorProfiles);

    if (SUCCEEDED(hr))
    {
        // 언어 프로필 제거
        pInputProcessorProfiles->RemoveLanguageProfile(c_clsidKRIME, 0x0412, c_guidProfile);

        // 텍스트 서비스 등록 해제
        pInputProcessorProfiles->Unregister(c_clsidKRIME);

        pInputProcessorProfiles->Release();
    }

    // 2. 카테고리 등록 해제
    ITfCategoryMgr* pCategoryMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_CategoryMgr,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfCategoryMgr,
        (void**)&pCategoryMgr);

    if (SUCCEEDED(hr))
    {
        pCategoryMgr->UnregisterCategory(c_clsidKRIME, GUID_TFCAT_TIP_KEYBOARD, c_clsidKRIME);
        pCategoryMgr->UnregisterCategory(c_clsidKRIME, GUID_TFCAT_TIPCAP_UIELEMENTENABLED, c_clsidKRIME);
        pCategoryMgr->UnregisterCategory(c_clsidKRIME, GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, c_clsidKRIME);
        pCategoryMgr->Release();
    }

    // 3. COM 서버 등록 해제
    WCHAR szClsid[MAX_PATH];
    if (StringFromGUID2(c_clsidKRIME, szClsid, ARRAYSIZE(szClsid)) > 0)
    {
        WCHAR szKeyPath[MAX_PATH];
        StringCchPrintfW(szKeyPath, MAX_PATH, L"CLSID\\%s", szClsid);
        SHDeleteKeyW(HKEY_CLASSES_ROOT, szKeyPath);
    }

    CoUninitialize();

    LOG_WRITE("UnregisterServer successful.");
    return S_OK;
}
