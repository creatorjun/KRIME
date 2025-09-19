#include "pch.h"
#include "LanguageBar.h"
#include "Global.h"
#include "resource.h"

static const WCHAR c_szLangBarItemDesc[] = L"KRIME Mode";
static const WCHAR c_szLangBarTooltip[] = L"Toggle Hangul/English";

// --- Constructor / Destructor ---
CLanguageBar::CLanguageBar()
{
    _refCount = 1;
    _pLangBarItemSink = nullptr;
    _status = TF_LBI_STATUS_BTN_TOGGLED; // 기본값 OFF (영문)

    _info.clsidService = CLSID_KRIME;
    _info.guidItem = GUID_LBI_KRIME_INPUTMODE;
    _info.dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY;
    _info.ulSort = 0;
    StringCchCopy(_info.szDescription, ARRAYSIZE(_info.szDescription), c_szLangBarItemDesc);
}

CLanguageBar::~CLanguageBar()
{
    if (_pLangBarItemSink)
    {
        _pLangBarItemSink->Release();
    }
}

// --- IUnknown Implementation ---
STDMETHODIMP CLanguageBar::QueryInterface(REFIID riid, void** ppvObj)
{
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItem) ||
        IsEqualIID(riid, IID_ITfLangBarItemButton))
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
STDMETHODIMP_(ULONG) CLanguageBar::AddRef(void)
{
    return ++_refCount;
}
STDMETHODIMP_(ULONG) CLanguageBar::Release(void)
{
    LONG cr = --_refCount;
    if (cr == 0) delete this;
    return cr;
}

// --- ITfLangBarItem Implementation ---
STDMETHODIMP CLanguageBar::GetInfo(TF_LANGBARITEMINFO* pInfo)
{
    if (!pInfo) return E_INVALIDARG;
    *pInfo = _info;
    return S_OK;
}
STDMETHODIMP CLanguageBar::GetStatus(DWORD* pdwStatus)
{
    if (!pdwStatus) return E_INVALIDARG;
    *pdwStatus = _status;
    return S_OK;
}
STDMETHODIMP CLanguageBar::Show(BOOL fShow)
{
    // The language bar will show/hide our item based on the return value.
    return S_OK;
}
STDMETHODIMP CLanguageBar::GetTooltipString(BSTR* pbstrToolTip)
{
    if (!pbstrToolTip) return E_INVALIDARG;
    *pbstrToolTip = SysAllocString(c_szLangBarTooltip);
    return (*pbstrToolTip == nullptr) ? E_OUTOFMEMORY : S_OK;
}

// --- ITfLangBarItemButton Implementation ---
STDMETHODIMP CLanguageBar::OnClick(TfLBIClick click, POINT pt, const RECT* prcArea)
{
    if (click == TF_LBI_CLK_LEFT)
    {
        // Toggle the status
        BOOL isToggled = (_status & TF_LBI_STATUS_BTN_TOGGLED);
        SetStatus(TF_LBI_STATUS_BTN_TOGGLED, !isToggled);

        // TODO: CTextService의 한/영 상태 변경 함수를 호출해야 함
    }
    return S_OK;
}
STDMETHODIMP CLanguageBar::InitMenu(ITfMenu* pMenu)
{
    // No context menu for now
    return S_OK;
}
STDMETHODIMP CLanguageBar::OnMenuSelect(UINT wID)
{
    return S_OK;
}
STDMETHODIMP CLanguageBar::GetIcon(HICON* phIcon)
{
    if (!phIcon) return E_INVALIDARG;

    BOOL isHangulModeOn = !(_status & TF_LBI_STATUS_BTN_TOGGLED);
    UINT iconId = isHangulModeOn ? IDI_ICON_HANGUL_ON : IDI_ICON_HANGUL_OFF;

    *phIcon = (HICON)LoadImage(g_hInstDll, MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    return (*phIcon) ? S_OK : E_FAIL;
}
STDMETHODIMP CLanguageBar::GetText(BSTR* pbstrText)
{
    if (!pbstrText) return E_INVALIDARG;
    *pbstrText = SysAllocString(_info.szDescription);
    return (*pbstrText) ? S_OK : E_OUTOFMEMORY;
}

// --- ITfSource Implementation ---
STDMETHODIMP CLanguageBar::AdviseSink(REFIID riid, IUnknown* punk, DWORD* pdwCookie)
{
    if (!IsEqualIID(IID_ITfLangBarItemSink, riid)) return CONNECT_E_CANNOTCONNECT;
    if (_pLangBarItemSink) return CONNECT_E_ADVISELIMIT;

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void**)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = nullptr;
        return E_NOINTERFACE;
    }

    *pdwCookie = 1; // Simple cookie
    return S_OK;
}
STDMETHODIMP CLanguageBar::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie != 1 || !_pLangBarItemSink) return CONNECT_E_NOCONNECTION;

    _pLangBarItemSink->Release();
    _pLangBarItemSink = nullptr;
    return S_OK;
}

// --- Public Method ---
void CLanguageBar::SetStatus(DWORD dwStatus, BOOL fSet)
{
    BOOL fChanged = FALSE;
    if (fSet)
    {
        if (!(_status & dwStatus))
        {
            _status |= dwStatus;
            fChanged = TRUE;
        }
    }
    else
    {
        if (_status & dwStatus)
        {
            _status &= ~dwStatus;
            fChanged = TRUE;
        }
    }

    if (fChanged && _pLangBarItemSink)
    {
        _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
    }
}