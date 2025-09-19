#include "pch.h"
#include "Register.h"
#include "Global.h"
#include "TextService.h" // CTextService 참조를 위해 추가
#include "resource.h"    // IDI_ICON_MAIN 참조를 위해 추가

#define CLSID_STRLEN 38  // "{...}" 형식의 GUID 문자열 길이

// --- 함수 프로토타입 ---
static BOOL RegisterTextService(const GUID& clsid, const WCHAR* description);
static void UnregisterTextService(const GUID& clsid);
static BOOL RegisterProfile(const GUID& clsid, const GUID& profileGuid, const WCHAR* description);
static void UnregisterProfile(const GUID& clsid, const GUID& profileGuid);
static BOOL RegisterCategories(const GUID& clsid);
static void UnregisterCategories(const GUID& clsid);
static HRESULT CLSIDToString(REFGUID refGUID, WCHAR* pch);
static LONG RecurseDeleteKey(HKEY hParentKey, LPCTSTR lpszKey);


// --- 상수 정의 ---
static const WCHAR c_szDescription[] = L"KRIME (Custom)";
static const WCHAR c_szModel[] = L"Apartment";
static const GUID c_guidCategories[] = {
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER
};
static const LANGID c_langid = MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT);


// --- DllRegisterServer ---
STDAPI DllRegisterServer(void)
{
    if (!RegisterTextService(CLSID_KRIME, c_szDescription) ||
        !RegisterProfile(CLSID_KRIME, GUID_PROFILE_KRIME, c_szDescription) ||
        !RegisterCategories(CLSID_KRIME))
    {
        DllUnregisterServer(); // 실패 시 롤백
        return E_FAIL;
    }
    return S_OK;
}

// --- DllUnregisterServer ---
STDAPI DllUnregisterServer(void)
{
    UnregisterCategories(CLSID_KRIME);
    UnregisterProfile(CLSID_KRIME, GUID_PROFILE_KRIME);
    UnregisterTextService(CLSID_KRIME);
    return S_OK;
}


// --- 구현 세부 ---
BOOL RegisterTextService(const GUID& clsid, const WCHAR* description)
{
    HKEY hKey, hSubKey;
    WCHAR szClsid[CLSID_STRLEN + 1];
    WCHAR szSubKey[MAX_PATH];
    WCHAR szFilePath[MAX_PATH];

    if (CLSIDToString(clsid, szClsid) != S_OK)
        return FALSE;

    swprintf_s(szSubKey, ARRAYSIZE(szSubKey), L"CLSID\\%s", szClsid);

    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return FALSE;
    // 경고 C4267 해결: (DWORD) 형변환 추가
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE*)description, (DWORD)((wcslen(description) + 1) * sizeof(WCHAR)));

    if (RegCreateKeyEx(hKey, L"InProcServer32", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL) == ERROR_SUCCESS)
    {
        GetModuleFileName(g_hInstDll, szFilePath, ARRAYSIZE(szFilePath));
        // 경고 C4267 해결: (DWORD) 형변환 추가
        RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (const BYTE*)szFilePath, (DWORD)((wcslen(szFilePath) + 1) * sizeof(WCHAR)));
        RegSetValueEx(hSubKey, L"ThreadingModel", 0, REG_SZ, (const BYTE*)c_szModel, (DWORD)((wcslen(c_szModel) + 1) * sizeof(WCHAR)));
        RegCloseKey(hSubKey);
    }
    RegCloseKey(hKey);
    return TRUE;
}

void UnregisterTextService(const GUID& clsid)
{
    WCHAR szClsid[CLSID_STRLEN + 1];
    WCHAR szSubKey[MAX_PATH];

    if (CLSIDToString(clsid, szClsid) != S_OK)
        return;

    swprintf_s(szSubKey, ARRAYSIZE(szSubKey), L"CLSID\\%s", szClsid);
    RecurseDeleteKey(HKEY_CLASSES_ROOT, szSubKey);
}

BOOL RegisterProfile(const GUID& clsid, const GUID& profileGuid, const WCHAR* description)
{
    ITfInputProcessorProfileMgr* pProfileMgr;
    if (CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr) != S_OK)
        return FALSE;

    WCHAR szIconFile[MAX_PATH];
    GetModuleFileName(g_hInstDll, szIconFile, ARRAYSIZE(szIconFile));
    // 경고 C4267 해결: (DWORD) 형변환 추가
    DWORD cchIconFile = (DWORD)wcslen(szIconFile);
    UINT uIconIndex = -IDI_ICON_MAIN; // 리소스 ID를 음수로 지정

    HRESULT hr = pProfileMgr->RegisterProfile(clsid, c_langid, profileGuid, description, (ULONG)wcslen(description), szIconFile, cchIconFile, uIconIndex, NULL, 0, TRUE, 0);

    pProfileMgr->Release();
    return (hr == S_OK);
}

void UnregisterProfile(const GUID& clsid, const GUID& profileGuid)
{
    ITfInputProcessorProfileMgr* pProfileMgr;
    if (CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr) == S_OK)
    {
        pProfileMgr->UnregisterProfile(clsid, c_langid, profileGuid, 0);
        pProfileMgr->Release();
    }
}

BOOL RegisterCategories(const GUID& clsid)
{
    ITfCategoryMgr* pCategoryMgr;
    if (CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr) != S_OK)
        return FALSE;

    for (int i = 0; i < ARRAYSIZE(c_guidCategories); i++)
    {
        pCategoryMgr->RegisterCategory(clsid, c_guidCategories[i], clsid);
    }

    pCategoryMgr->Release();
    return TRUE;
}

void UnregisterCategories(const GUID& clsid)
{
    ITfCategoryMgr* pCategoryMgr;
    if (CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr) == S_OK)
    {
        for (int i = 0; i < ARRAYSIZE(c_guidCategories); i++)
        {
            pCategoryMgr->UnregisterCategory(clsid, c_guidCategories[i], clsid);
        }
        pCategoryMgr->Release();
    }
}


// --- Helper Functions ---
HRESULT CLSIDToString(REFGUID refGUID, WCHAR* pch)
{
    return StringFromGUID2(refGUID, pch, CLSID_STRLEN + 1) ? S_OK : E_FAIL;
}

LONG RecurseDeleteKey(HKEY hParentKey, LPCTSTR lpszKey)
{
    HKEY hKey;
    if (RegOpenKeyEx(hParentKey, lpszKey, 0, KEY_READ | DELETE, &hKey) != ERROR_SUCCESS)
        return ERROR_FILE_NOT_FOUND;

    WCHAR szSubKey[256];
    DWORD dwSubKeyLen = ARRAYSIZE(szSubKey);
    while (RegEnumKeyEx(hKey, 0, szSubKey, &dwSubKeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        RecurseDeleteKey(hKey, szSubKey);
        dwSubKeyLen = ARRAYSIZE(szSubKey);
    }

    RegCloseKey(hKey);
    return RegDeleteKey(hParentKey, lpszKey);
}