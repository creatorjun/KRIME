#include <ole2.h>
#include <olectl.h>  // Connection Point 오류 상수들을 위해 추가
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

    // 리소스로부터 아이콘 로드
    _hIconMain = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    _hIconKor = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_KOR), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    _hIconEng = (HICON)LoadImage(g_hModule, MAKEINTRESOURCE(IDI_ENG), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

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

// ITfLangBarItem
STDMETHODIMP CLangBarItem::GetInfo(TF_LANGBARITEMINFO* pInfo)
{
    if (pInfo == nullptr) return E_INVALIDARG;

    pInfo->clsidService = c_clsidKRIME;
    pInfo->guidItem = c_guidProfile; // 프로필 GUID를 아이템 ID로 사용
    pInfo->dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY;
    pInfo->ulSort = 0;
    StringCchCopy(pInfo->szDescription, ARRAYSIZE(pInfo->szDescription), L"KRIME 한/영 전환");

    return S_OK;
}

STDMETHODIMP CLangBarItem::GetStatus(DWORD* pdwStatus)
{
    if (pdwStatus == nullptr) return E_INVALIDARG;
    *pdwStatus = 0;
    return S_OK;
}

STDMETHODIMP CLangBarItem::Show(BOOL fShow)
{
    return E_NOTIMPL; // 시스템이 알아서 관리
}

STDMETHODIMP CLangBarItem::GetTooltipString(BSTR* pbstrToolTip)
{
    if (pbstrToolTip == nullptr) return E_INVALIDARG;
    *pbstrToolTip = SysAllocString(_isKoreanMode ? L"현재 한글 입력 상태입니다." : L"현재 영문 입력 상태입니다.");
    return (*pbstrToolTip == nullptr) ? E_OUTOFMEMORY : S_OK;
}

// ITfLangBarItemButton
STDMETHODIMP CLangBarItem::OnClick(TfLBIClick click, POINT pt, const RECT* prcArea)
{
    // 아이콘을 클릭해도 언어가 토글되도록 함
    _pTIP->ToggleLanguage();
    return S_OK;
}

STDMETHODIMP CLangBarItem::InitMenu(ITfMenu* pMenu) { return E_NOTIMPL; }
STDMETHODIMP CLangBarItem::OnMenuSelect(UINT wID) { return E_NOTIMPL; }

STDMETHODIMP CLangBarItem::GetIcon(HICON* phIcon)
{
    if (phIcon == nullptr) return E_INVALIDARG;
    *phIcon = _isKoreanMode ? _hIconKor : _hIconEng;
    // 초기에는 _hIconMain을 보여주고 싶다면 별도 로직 추가 가능
    return S_OK;
}

STDMETHODIMP CLangBarItem::GetText(BSTR* pbstrText)
{
    if (pbstrText == nullptr) return E_INVALIDARG;
    *pbstrText = SysAllocString(L""); // 아이콘만 표시하고 텍스트는 표시 안함
    return S_OK;
}

// --- ITfSource ---
// 이 함수를 아래 코드로 완전히 교체해주세요.
STDMETHODIMP CLangBarItem::AdviseSink(REFIID riid, IUnknown* punk, DWORD* pdwCookie)
{
    if (!IsEqualIID(riid, IID_ITfLangBarItemSink))
    {
        return CONNECT_E_CANNOTCONNECT; // if문 뒤에 세미콜론 제거
    }
    if (_pLangBarItemSink != nullptr)
    {
        return CONNECT_E_ADVISELIMIT; // if문 뒤에 세미콜론 제거
    }

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void**)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = nullptr;
        return E_NOINTERFACE;
    }

    *pdwCookie = 1; // 간단하게 1로 고정
    _dwSinkCookie = *pdwCookie;
    return S_OK;
}

// 이 함수를 아래 코드로 완전히 교체해주세요.
STDMETHODIMP CLangBarItem::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie != _dwSinkCookie || _pLangBarItemSink == nullptr)
    {
        return CONNECT_E_NOCONNECTION; // if문 뒤에 세미콜론 제거
    }

    _pLangBarItemSink->Release();
    _pLangBarItemSink = nullptr;
    _dwSinkCookie = TF_INVALID_COOKIE;
    return S_OK;
}

// Public method to update the icon
void CLangBarItem::UpdateIcon(BOOL isKorean)
{
    _isKoreanMode = isKorean;
    if (_pLangBarItemSink)
    {
        // TSF 관리자에게 아이콘 및 툴팁을 업데이트하라고 알림
        _pLangBarItemSink->OnUpdate(TF_LBI_ICON | TF_LBI_TOOLTIP);
    }
}