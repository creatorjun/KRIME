#include "framework.h"
#include "Register.h"

#define TEXT_SERVICE_NAME L"KRIME Text Service"
#define TEXT_SERVICE_PROFILE_NAME L"KRIME �ѱ� �Է±�"

// --- �Լ� ���� ���� (Forward Declarations) ---
static BOOL SetRegistryKey(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, const WCHAR* data);
static BOOL DeleteRegistryTree(HKEY hKey, LPCWSTR subKey);

// --- ���� ��� ---
HRESULT RegisterServer()
{
    LOG_WRITE("RegisterServer started.");

    WCHAR szClsid[MAX_PATH];
    if (StringFromGUID2(c_clsidKRIME, szClsid, ARRAYSIZE(szClsid)) == 0) return E_FAIL;

    WCHAR szProfileGuid[MAX_PATH];
    if (StringFromGUID2(c_guidProfile, szProfileGuid, ARRAYSIZE(szProfileGuid)) == 0) return E_FAIL;

    WCHAR szModulePath[MAX_PATH];
    if (GetModuleFileNameW(g_hModule, szModulePath, ARRAYSIZE(szModulePath)) == 0) return E_FAIL;

    WCHAR szKeyPath[MAX_PATH];
    WCHAR szCategoryGuid[MAX_PATH];
    WCHAR szIconIndex[32];

    // 1. CLSID ���
    StringCchPrintfW(szKeyPath, MAX_PATH, L"CLSID\\%s", szClsid);
    if (!SetRegistryKey(HKEY_CLASSES_ROOT, szKeyPath, NULL, TEXT_SERVICE_NAME)) return E_FAIL;
    LOG_WRITE("Registered CLSID.");

    // 2. InprocServer32 ���
    StringCchPrintfW(szKeyPath, MAX_PATH, L"CLSID\\%s\\InprocServer32", szClsid);
    if (!SetRegistryKey(HKEY_CLASSES_ROOT, szKeyPath, NULL, szModulePath)) return E_FAIL;
    if (!SetRegistryKey(HKEY_CLASSES_ROOT, szKeyPath, L"ThreadingModel", L"Apartment")) return E_FAIL;
    LOG_WRITE("Registered InprocServer32.");

    // 3. TSF ������ ���
    StringCchPrintfW(szKeyPath, MAX_PATH, L"SOFTWARE\\Microsoft\\CTF\\TIP\\%s\\LanguageProfile\\0x0412\\%s", szClsid, szProfileGuid);
    if (!SetRegistryKey(HKEY_LOCAL_MACHINE, szKeyPath, NULL, TEXT_SERVICE_PROFILE_NAME)) return E_FAIL;
    if (!SetRegistryKey(HKEY_LOCAL_MACHINE, szKeyPath, L"IconFile", szModulePath)) return E_FAIL;
    StringCchPrintfW(szIconIndex, ARRAYSIZE(szIconIndex), L"%d", IDI_MAIN);
    if (!SetRegistryKey(HKEY_LOCAL_MACHINE, szKeyPath, L"IconIndex", szIconIndex)) return E_FAIL;
    LOG_WRITE("Registered TSF Language Profile with icon.");

    // 4. ī�װ� ��� (������ �κ�)
    // GUID�� �� �̸��� �ƴ�, Ű ����� �Ϻη� ����� ����մϴ�.
    if (StringFromGUID2(GUID_TFCAT_TIP_KEYBOARD, szCategoryGuid, ARRAYSIZE(szCategoryGuid)) == 0) return E_FAIL;
    StringCchPrintfW(szKeyPath, MAX_PATH, L"SOFTWARE\\Microsoft\\CTF\\TIP\\%s\\Category\\%s", szClsid, szCategoryGuid);
    if (!SetRegistryKey(HKEY_LOCAL_MACHINE, szKeyPath, NULL, NULL)) return E_FAIL;

    if (StringFromGUID2(GUID_TFCAT_TIPCAP_UIELEMENTENABLED, szCategoryGuid, ARRAYSIZE(szCategoryGuid)) == 0) return E_FAIL;
    StringCchPrintfW(szKeyPath, MAX_PATH, L"SOFTWARE\\Microsoft\\CTF\\TIP\\%s\\Category\\%s", szClsid, szCategoryGuid);
    if (!SetRegistryKey(HKEY_LOCAL_MACHINE, szKeyPath, NULL, NULL)) return E_FAIL;

    LOG_WRITE("Registered TSF categories correctly.");

    LOG_WRITE("RegisterServer successful.");
    return S_OK;
}

// --- ���� ��� ��� ---
HRESULT UnregisterServer()
{
    LOG_WRITE("UnregisterServer started.");

    WCHAR szClsid[MAX_PATH];
    if (StringFromGUID2(c_clsidKRIME, szClsid, ARRAYSIZE(szClsid)) == 0) return E_FAIL;

    WCHAR szKeyPath[MAX_PATH];

    StringCchPrintfW(szKeyPath, MAX_PATH, L"SOFTWARE\\Microsoft\\CTF\\TIP\\%s", szClsid);
    DeleteRegistryTree(HKEY_LOCAL_MACHINE, szKeyPath);
    LOG_WRITE("Deleted TSF profile keys.");

    StringCchPrintfW(szKeyPath, MAX_PATH, L"CLSID\\%s", szClsid);
    DeleteRegistryTree(HKEY_CLASSES_ROOT, szKeyPath);
    LOG_WRITE("Deleted CLSID keys.");

    LOG_WRITE("UnregisterServer successful.");
    return S_OK;
}


// --- ����� �Լ� ���� ---
static BOOL SetRegistryKey(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, const WCHAR* data)
{
    HKEY hSubKey;
    if (RegCreateKeyExW(hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL) != ERROR_SUCCESS) return FALSE;

    if (data)
    {
        DWORD cbData = static_cast<DWORD>((wcslen(data) + 1) * sizeof(WCHAR));
        if (RegSetValueExW(hSubKey, valueName, 0, REG_SZ, (const BYTE*)data, cbData) != ERROR_SUCCESS)
        {
            RegCloseKey(hSubKey);
            return FALSE;
        }
    }
    RegCloseKey(hSubKey);
    return TRUE;
}

static BOOL DeleteRegistryTree(HKEY hKey, LPCWSTR subKey)
{
    return SHDeleteKeyW(hKey, subKey) == ERROR_SUCCESS;
}