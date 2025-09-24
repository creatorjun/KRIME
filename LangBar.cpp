// LangBar.cpp - ������ ����
#include <ole2.h>
#include <olectl.h>
#include <strsafe.h>
#include "LangBar.h"
#include "TextInputProcessor.h"
#include "Globals.h"

CLangBarItem::CLangBarItem(CTextInputProcessor* pTIP)
{
    _cRef = 1;
    _pTIP = pTIP;
    _pTIP->AddRef();
    _pLangBarItemSink = nullptr;
    _dwSinkCookie = TF_INVALID_COOKIE;
    _isKoreanMode = TRUE; // �⺻��: �ѱ� ���

    // �ý��� ������ ũ�� ���
    int iconWidth = GetSystemMetrics(SM_CXSMICON);
    int iconHeight = GetSystemMetrics(SM_CYSMICON);

    // ���ҽ��κ��� ������ �ε� (ũ�� ����)
    _hIconMain = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR);
    _hIconKor = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_KOR), IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR);
    _hIconEng = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_ENG), IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR);

    // ������ �ε� ���� �� �α� ���
    LOG_WRITE("Icon loading - Main: %s, Kor: %s, Eng: %s",
        _hIconMain ? "OK" : "FAILED",
        _hIconKor ? "OK" : "FAILED",
        _hIconEng ? "OK" : "FAILED");

    g_cRefDll++;
}

CLangBarItem::~CLangBarItem()
{
    if (_pTIP) _pTIP->Release();
    if (_hIconMain) DestroyIcon(_hIconMain);
    if (_hIconKor) DestroyIcon(_hIconKor);
    if (_hIconEng) DestroyIcon(_hIconEng);
    g_cRefDll--;
}

STDMETHODIMP CLangBarItem::QueryInterface(REFIID riid, void** ppvObj)
{
    if (ppvObj == nullptr) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfLangBarItem) || IsEqualIID(riid, IID_ITfLangBarItemButton))
    {
        *ppvObj = (ITfLangBarItemButton*)this;
    }
    else if (IsEqualIID(riid, IID_ITfSource))
    {
        *ppvObj = (ITfSource*)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CLangBarItem::AddRef() { return ++_cRef; }
STDMETHODIMP_(ULONG) CLangBarItem::Release()
{
    long cr = --_cRef;
    if (cr == 0) delete this;
    return cr;
}

STDMETHODIMP CLangBarItem::GetInfo(TF_LANGBARITEMINFO* pInfo)
{
    if (pInfo == nullptr) return E_INVALIDARG;

    pInfo->clsidService = c_clsidKRIME;
    pInfo->guidItem = c_guidProfile;

    // ���� ������ �κ�: HIDDENBYDEFAULT ���� ����
    pInfo->dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY;
    // TF_LBI_STYLE_HIDDENBYDEFAULT ������
    // ���� ������� ���� ����

    pInfo->ulSort = 0;
    StringCchCopy(pInfo->szDescription, ARRAYSIZE(pInfo->szDescription), L"KRIME ��/�� ��ȯ");

    LOG_WRITE("GetInfo called - Style: 0x%08X", pInfo->dwStyle);
    return S_OK;
}

STDMETHODIMP CLangBarItem::GetStatus(DWORD* pdwStatus)
{
    if (pdwStatus == nullptr) return E_INVALIDARG;

    // �������� ��������� ǥ���ϵ��� ����
    *pdwStatus = 0; // TF_LBI_STATUS_HIDDEN ����

    LOG_WRITE("GetStatus called - Status: 0x%08X", *pdwStatus);
    return S_OK;
}


STDMETHODIMP CLangBarItem::Show(BOOL fShow)
{
    return E_NOTIMPL; // �ý����� �˾Ƽ� ����
}

STDMETHODIMP CLangBarItem::GetTooltipString(BSTR* pbstrToolTip)
{
    if (pbstrToolTip == nullptr) return E_INVALIDARG;
    *pbstrToolTip = SysAllocString(_isKoreanMode ? L"�ѱ� �Է� ���" : L"���� �Է� ���");
    return (*pbstrToolTip == nullptr) ? E_OUTOFMEMORY : S_OK;
}

// ITfLangBarItemButton
STDMETHODIMP CLangBarItem::OnClick(TfLBIClick click, POINT pt, const RECT* prcArea)
{
    LOG_WRITE("LangBar button clicked");
    _pTIP->ToggleLanguage();
    return S_OK;
}

STDMETHODIMP CLangBarItem::InitMenu(ITfMenu* pMenu) { return E_NOTIMPL; }
STDMETHODIMP CLangBarItem::OnMenuSelect(UINT wID) { return E_NOTIMPL; }

// ���� GetIcon �޼��� ������ ���� ����
STDMETHODIMP CLangBarItem::GetIcon(HICON* phIcon)
{
    if (phIcon == nullptr) return E_INVALIDARG;

    HICON hSrcIcon = nullptr;

    // ���� ��忡 ���� �ҽ� ������ ����
    if (_isKoreanMode)
    {
        hSrcIcon = _hIconKor;
        LOG_WRITE("GetIcon - Korean mode selected");
    }
    else
    {
        hSrcIcon = _hIconEng;
        LOG_WRITE("GetIcon - English mode selected");
    }

    // �������� ���� ��� ���� ������ ���
    if (hSrcIcon == nullptr)
    {
        hSrcIcon = _hIconMain;
        LOG_WRITE("GetIcon - Using main icon as fallback");
    }

    // ������ �������� ���� ���
    if (hSrcIcon == nullptr)
    {
        LOG_WRITE("GetIcon - No icon available, returning NULL");
        *phIcon = nullptr;
        return S_OK; // NULL �����ܵ� ��ȿ�� ��ȯ���Դϴ�
    }

    // ���� �߿�: ������ ���纻 ���� (�ý����� �����ϹǷ�) ����
    *phIcon = CopyIcon(hSrcIcon);

    if (*phIcon == nullptr)
    {
        LOG_WRITE("GetIcon - Failed to copy icon");
        return E_FAIL;
    }

    LOG_WRITE("GetIcon - Successfully returned icon copy");
    return S_OK;
}
// ���� GetIcon �޼��� ���� �Ϸ� ����

// ���� GetText �޼��� ���� ����
STDMETHODIMP CLangBarItem::GetText(BSTR* pbstrText)
{
    if (pbstrText == nullptr) return E_INVALIDARG;

    // �������� �ִ� ��� �ؽ�Ʈ�� ����ΰ�, ���� ��쿡�� �ؽ�Ʈ ǥ��
    if ((_isKoreanMode && _hIconKor) || (!_isKoreanMode && _hIconEng) || _hIconMain)
    {
        *pbstrText = SysAllocString(L""); // �������� ������ �ؽ�Ʈ ���
        LOG_WRITE("GetText - Returning empty text (icon available)");
    }
    else
    {
        // �������� ���� ��쿡�� �ؽ�Ʈ ǥ��
        *pbstrText = SysAllocString(_isKoreanMode ? L"��" : L"A");
        LOG_WRITE("GetText - Returning fallback text: %s", _isKoreanMode ? "��" : "A");
    }

    return (*pbstrText == nullptr) ? E_OUTOFMEMORY : S_OK;
}
// ���� GetText �޼��� ���� �Ϸ� ����

// ITfSource (���� ����)
STDMETHODIMP CLangBarItem::AdviseSink(REFIID riid, IUnknown* punk, DWORD* pdwCookie)
{
    if (!IsEqualIID(riid, IID_ITfLangBarItemSink))
        return CONNECT_E_CANNOTCONNECT;

    if (_pLangBarItemSink != nullptr)
        return CONNECT_E_ADVISELIMIT;

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void**)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = nullptr;
        return E_NOINTERFACE;
    }

    *pdwCookie = 1;
    _dwSinkCookie = *pdwCookie;
    return S_OK;
}

STDMETHODIMP CLangBarItem::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie != _dwSinkCookie || _pLangBarItemSink == nullptr)
        return CONNECT_E_NOCONNECTION;

    _pLangBarItemSink->Release();
    _pLangBarItemSink = nullptr;
    _dwSinkCookie = TF_INVALID_COOKIE;
    return S_OK;
}

// Public method to update the icon
void CLangBarItem::UpdateIcon(BOOL isKorean)
{
    _isKoreanMode = isKorean;
    LOG_WRITE("UpdateIcon called - Mode: %s", isKorean ? "Korean" : "English");

    if (_pLangBarItemSink)
    {
        // TSF �����ڿ��� ������ �� �ؽ�Ʈ�� ������Ʈ�϶�� �˸�
        _pLangBarItemSink->OnUpdate(TF_LBI_ICON | TF_LBI_TEXT | TF_LBI_TOOLTIP);
        LOG_WRITE("OnUpdate called with TF_LBI_ICON | TF_LBI_TEXT | TF_LBI_TOOLTIP");
    }
    else
    {
        LOG_WRITE("UpdateIcon - No sink available");
    }
}
