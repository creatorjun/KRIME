// LangBar.cpp - 수정된 버전
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
    _isKoreanMode = TRUE; // 기본값: 한글 모드

    // 시스템 아이콘 크기 얻기
    int iconWidth = GetSystemMetrics(SM_CXSMICON);
    int iconHeight = GetSystemMetrics(SM_CYSMICON);

    // 리소스로부터 아이콘 로드 (크기 지정)
    _hIconMain = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR);
    _hIconKor = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_KOR), IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR);
    _hIconEng = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_ENG), IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR);

    // 아이콘 로딩 실패 시 로그 출력
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

    // ▼▼▼ 수정된 부분: HIDDENBYDEFAULT 제거 ▼▼▼
    pInfo->dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY;
    // TF_LBI_STYLE_HIDDENBYDEFAULT 제거함
    // ▲▲▲ 여기까지 수정 ▲▲▲

    pInfo->ulSort = 0;
    StringCchCopy(pInfo->szDescription, ARRAYSIZE(pInfo->szDescription), L"KRIME 한/영 전환");

    LOG_WRITE("GetInfo called - Style: 0x%08X", pInfo->dwStyle);
    return S_OK;
}

STDMETHODIMP CLangBarItem::GetStatus(DWORD* pdwStatus)
{
    if (pdwStatus == nullptr) return E_INVALIDARG;

    // 아이템을 명시적으로 표시하도록 설정
    *pdwStatus = 0; // TF_LBI_STATUS_HIDDEN 제거

    LOG_WRITE("GetStatus called - Status: 0x%08X", *pdwStatus);
    return S_OK;
}


STDMETHODIMP CLangBarItem::Show(BOOL fShow)
{
    return E_NOTIMPL; // 시스템이 알아서 관리
}

STDMETHODIMP CLangBarItem::GetTooltipString(BSTR* pbstrToolTip)
{
    if (pbstrToolTip == nullptr) return E_INVALIDARG;
    *pbstrToolTip = SysAllocString(_isKoreanMode ? L"한글 입력 모드" : L"영문 입력 모드");
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

// ▼▼▼ GetIcon 메서드 완전히 수정 ▼▼▼
STDMETHODIMP CLangBarItem::GetIcon(HICON* phIcon)
{
    if (phIcon == nullptr) return E_INVALIDARG;

    HICON hSrcIcon = nullptr;

    // 현재 모드에 따라 소스 아이콘 선택
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

    // 아이콘이 없는 경우 메인 아이콘 사용
    if (hSrcIcon == nullptr)
    {
        hSrcIcon = _hIconMain;
        LOG_WRITE("GetIcon - Using main icon as fallback");
    }

    // 여전히 아이콘이 없는 경우
    if (hSrcIcon == nullptr)
    {
        LOG_WRITE("GetIcon - No icon available, returning NULL");
        *phIcon = nullptr;
        return S_OK; // NULL 아이콘도 유효한 반환값입니다
    }

    // ▼▼▼ 중요: 아이콘 복사본 생성 (시스템이 해제하므로) ▼▼▼
    *phIcon = CopyIcon(hSrcIcon);

    if (*phIcon == nullptr)
    {
        LOG_WRITE("GetIcon - Failed to copy icon");
        return E_FAIL;
    }

    LOG_WRITE("GetIcon - Successfully returned icon copy");
    return S_OK;
}
// ▲▲▲ GetIcon 메서드 수정 완료 ▲▲▲

// ▼▼▼ GetText 메서드 수정 ▼▼▼
STDMETHODIMP CLangBarItem::GetText(BSTR* pbstrText)
{
    if (pbstrText == nullptr) return E_INVALIDARG;

    // 아이콘이 있는 경우 텍스트는 비워두고, 없는 경우에만 텍스트 표시
    if ((_isKoreanMode && _hIconKor) || (!_isKoreanMode && _hIconEng) || _hIconMain)
    {
        *pbstrText = SysAllocString(L""); // 아이콘이 있으면 텍스트 비움
        LOG_WRITE("GetText - Returning empty text (icon available)");
    }
    else
    {
        // 아이콘이 없는 경우에만 텍스트 표시
        *pbstrText = SysAllocString(_isKoreanMode ? L"한" : L"A");
        LOG_WRITE("GetText - Returning fallback text: %s", _isKoreanMode ? "한" : "A");
    }

    return (*pbstrText == nullptr) ? E_OUTOFMEMORY : S_OK;
}
// ▲▲▲ GetText 메서드 수정 완료 ▲▲▲

// ITfSource (변경 없음)
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
        // TSF 관리자에게 아이콘 및 텍스트를 업데이트하라고 알림
        _pLangBarItemSink->OnUpdate(TF_LBI_ICON | TF_LBI_TEXT | TF_LBI_TOOLTIP);
        LOG_WRITE("OnUpdate called with TF_LBI_ICON | TF_LBI_TEXT | TF_LBI_TOOLTIP");
    }
    else
    {
        LOG_WRITE("UpdateIcon - No sink available");
    }
}
