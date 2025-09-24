// Register.cpp - ������ ����
#include "framework.h"
#include "Register.h"
#include <msctf.h>

#define TEXT_SERVICE_NAME L"KRIME Text Service"
#define TEXT_SERVICE_PROFILE_NAME L"KRIME �ѱ� �Է±�"

// --- COM ���� ��� (���� ��� ����) ---
static BOOL RegisterCOMServer()
{
    LOG_WRITE("RegisterCOMServer started.");

    WCHAR szClsid[MAX_PATH];
    if (StringFromGUID2(c_clsidKRIME, szClsid, ARRAYSIZE(szClsid)) == 0) return FALSE;

    WCHAR szModulePath[MAX_PATH];
    if (GetModuleFileNameW(g_hModule, szModulePath, ARRAYSIZE(szModulePath)) == 0) return FALSE;

    WCHAR szKeyPath[MAX_PATH];

    // 1. CLSID ���
    StringCchPrintfW(szKeyPath, MAX_PATH, L"CLSID\\%s", szClsid);

    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, szKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return FALSE;

    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)TEXT_SERVICE_NAME, (DWORD)((wcslen(TEXT_SERVICE_NAME) + 1) * sizeof(WCHAR)));
    RegCloseKey(hKey);

    // 2. InprocServer32 ���
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

// --- TSF ��� (�ùٸ� ���) ---
static BOOL RegisterTSF()
{
    LOG_WRITE("RegisterTSF started.");

    HRESULT hr;

    // 1. ITfInputProcessorProfiles ����
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

    // 2. �ؽ�Ʈ ���� ���
    hr = pInputProcessorProfiles->Register(c_clsidKRIME);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to register text service. HRESULT = 0x%08X", hr);
        pInputProcessorProfiles->Release();
        return FALSE;
    }

    // 3. ��� ������ �߰� (�ѱ��� - 0x0412)
    WCHAR szModulePath[MAX_PATH];
    GetModuleFileNameW(g_hModule, szModulePath, ARRAYSIZE(szModulePath));

    hr = pInputProcessorProfiles->AddLanguageProfile(c_clsidKRIME,
        0x0412,  // �ѱ��� LANGID
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

    // 4. ITfCategoryMgr�� ī�װ� ���
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

    // Ű���� TIP ī�װ� ���
    hr = pCategoryMgr->RegisterCategory(c_clsidKRIME, GUID_TFCAT_TIP_KEYBOARD, c_clsidKRIME);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to register keyboard category. HRESULT = 0x%08X", hr);
    }

    // UI ��� ���� ī�װ� ���
    hr = pCategoryMgr->RegisterCategory(c_clsidKRIME, GUID_TFCAT_TIPCAP_UIELEMENTENABLED, c_clsidKRIME);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to register UI element category. HRESULT = 0x%08X", hr);
    }

    // Windows �� ȣȯ�� ī�װ� ��� (Windows 8 �̻�)
    hr = pCategoryMgr->RegisterCategory(c_clsidKRIME, GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, c_clsidKRIME);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to register immersive support category. HRESULT = 0x%08X", hr);
    }

    pCategoryMgr->Release();

    LOG_WRITE("RegisterTSF successful.");
    return TRUE;
}

// --- ���� ��� �Լ� ---
HRESULT RegisterServer()
{
    LOG_WRITE("RegisterServer started.");

    // COM �ʱ�ȭ
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        LOG_WRITE("Failed to initialize COM. HRESULT = 0x%08X", hr);
        return hr;
    }

    BOOL success = TRUE;

    // 1. COM ���� ���
    if (!RegisterCOMServer())
    {
        LOG_WRITE("RegisterCOMServer failed.");
        success = FALSE;
    }

    // 2. TSF ���
    if (success && !RegisterTSF())
    {
        LOG_WRITE("RegisterTSF failed.");
        success = FALSE;
    }

    CoUninitialize();

    LOG_WRITE("RegisterServer %s.", success ? "successful" : "failed");
    return success ? S_OK : E_FAIL;
}

// --- ��� ���� ---
HRESULT UnregisterServer()
{
    LOG_WRITE("UnregisterServer started.");

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return hr;

    BOOL success = TRUE;

    // 1. TSF ��� ����
    ITfInputProcessorProfiles* pInputProcessorProfiles = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles,
        (void**)&pInputProcessorProfiles);

    if (SUCCEEDED(hr))
    {
        // ��� ������ ����
        pInputProcessorProfiles->RemoveLanguageProfile(c_clsidKRIME, 0x0412, c_guidProfile);

        // �ؽ�Ʈ ���� ��� ����
        pInputProcessorProfiles->Unregister(c_clsidKRIME);

        pInputProcessorProfiles->Release();
    }

    // 2. ī�װ� ��� ����
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

    // 3. COM ���� ��� ����
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
