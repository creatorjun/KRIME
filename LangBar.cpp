#include <ole2.h>
#include <olectl.h>  // Connection Point ���� ������� ���� �߰�
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

    // ���ҽ��κ��� ������ �ε�
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
    pInfo->guidItem = c_guidProfile; // ������ GUID�� ������ ID�� ���
    pInfo->dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY;
    pInfo->ulSort = 0;
    StringCchCopy(pInfo->szDescription, ARRAYSIZE(pInfo->szDescription), L"KRIME ��/�� ��ȯ");

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
    return E_NOTIMPL; // �ý����� �˾Ƽ� ����
}

STDMETHODIMP CLangBarItem::GetTooltipString(BSTR* pbstrToolTip)
{
    if (pbstrToolTip == nullptr) return E_INVALIDARG;
    *pbstrToolTip = SysAllocString(_isKoreanMode ? L"���� �ѱ� �Է� �����Դϴ�." : L"���� ���� �Է� �����Դϴ�.");
    return (*pbstrToolTip == nullptr) ? E_OUTOFMEMORY : S_OK;
}

// ITfLangBarItemButton
STDMETHODIMP CLangBarItem::OnClick(TfLBIClick click, POINT pt, const RECT* prcArea)
{
    // �������� Ŭ���ص� �� ��۵ǵ��� ��
    _pTIP->ToggleLanguage();
    return S_OK;
}

STDMETHODIMP CLangBarItem::InitMenu(ITfMenu* pMenu) { return E_NOTIMPL; }
STDMETHODIMP CLangBarItem::OnMenuSelect(UINT wID) { return E_NOTIMPL; }

STDMETHODIMP CLangBarItem::GetIcon(HICON* phIcon)
{
    if (phIcon == nullptr) return E_INVALIDARG;
    *phIcon = _isKoreanMode ? _hIconKor : _hIconEng;
    // �ʱ⿡�� _hIconMain�� �����ְ� �ʹٸ� ���� ���� �߰� ����
    return S_OK;
}

STDMETHODIMP CLangBarItem::GetText(BSTR* pbstrText)
{
    if (pbstrText == nullptr) return E_INVALIDARG;
    *pbstrText = SysAllocString(L""); // �����ܸ� ǥ���ϰ� �ؽ�Ʈ�� ǥ�� ����
    return S_OK;
}

// --- ITfSource ---
// �� �Լ��� �Ʒ� �ڵ�� ������ ��ü���ּ���.
STDMETHODIMP CLangBarItem::AdviseSink(REFIID riid, IUnknown* punk, DWORD* pdwCookie)
{
    if (!IsEqualIID(riid, IID_ITfLangBarItemSink))
    {
        return CONNECT_E_CANNOTCONNECT; // if�� �ڿ� �����ݷ� ����
    }
    if (_pLangBarItemSink != nullptr)
    {
        return CONNECT_E_ADVISELIMIT; // if�� �ڿ� �����ݷ� ����
    }

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void**)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = nullptr;
        return E_NOINTERFACE;
    }

    *pdwCookie = 1; // �����ϰ� 1�� ����
    _dwSinkCookie = *pdwCookie;
    return S_OK;
}

// �� �Լ��� �Ʒ� �ڵ�� ������ ��ü���ּ���.
STDMETHODIMP CLangBarItem::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie != _dwSinkCookie || _pLangBarItemSink == nullptr)
    {
        return CONNECT_E_NOCONNECTION; // if�� �ڿ� �����ݷ� ����
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
        // TSF �����ڿ��� ������ �� ������ ������Ʈ�϶�� �˸�
        _pLangBarItemSink->OnUpdate(TF_LBI_ICON | TF_LBI_TOOLTIP);
    }
}