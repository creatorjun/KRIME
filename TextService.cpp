#include "pch.h"
#include "TextService.h"
#include "LanguageBar.h"


// --- Constructor / Destructor ---
CTextService::CTextService()
{
    _refCount = 1;
    _pThreadMgr = nullptr;
    _tfClientId = TF_CLIENTID_NULL;
    _pComposition = nullptr;
    _pLangBar = nullptr;
    _isHangulMode = FALSE;
}

CTextService::~CTextService()
{
    // _pLangBar는 Deactivate에서 RemoveItem 되면서 해제되므로
    // 여기서 별도로 해제할 필요가 없습니다.
}

// --- Static Factory Method ---
HRESULT CTextService::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj)
{
    if (ppvObj == nullptr) return E_INVALIDARG;
    *ppvObj = nullptr;
    if (pUnkOuter != nullptr) return CLASS_E_NOAGGREGATION;
    CTextService* pTextService = new CTextService();
    if (pTextService == nullptr) return E_OUTOFMEMORY;
    HRESULT hr = pTextService->QueryInterface(riid, ppvObj);
    pTextService->Release();
    return hr;
}

// --- IUnknown Implementation ---
STDMETHODIMP CTextService::QueryInterface(REFIID riid, void** ppvObj)
{
    if (ppvObj == nullptr) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor))
    {
        *ppvObj = (ITfTextInputProcessor*)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx))
    {
        *ppvObj = (ITfTextInputProcessorEx*)this;
    }
    else if (IsEqualIID(riid, IID_ITfKeyEventSink))
    {
        *ppvObj = (ITfKeyEventSink*)this;
    }
    else if (IsEqualIID(riid, IID_ITfCompositionSink)) // ITfCompositionSink 추가
    {
        *ppvObj = (ITfCompositionSink*)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}
STDMETHODIMP_(ULONG) CTextService::AddRef(void) { return ++_refCount; }
STDMETHODIMP_(ULONG) CTextService::Release(void)
{
    LONG cr = --_refCount;
    if (_refCount == 0) delete this;
    return cr;
}

// --- ITfTextInputProcessor / ITfTextInputProcessorEx Implementation ---
STDMETHODIMP CTextService::Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId)
{
    return this->ActivateEx(pThreadMgr, tfClientId, 0);
}

STDMETHODIMP CTextService::Deactivate()
{
    // 반드시 초기화의 역순으로 해제합니다.
    _UninitLanguageBar();
    _UninitKeyEventSink();

    if (_pThreadMgr)
    {
        _pThreadMgr->Release();
        _pThreadMgr = nullptr;
    }
    _tfClientId = TF_CLIENTID_NULL;
    return S_OK;
}

STDMETHODIMP CTextService::ActivateEx(ITfThreadMgr* pThreadMgr, TfClientId tfClientId, DWORD dwFlags)
{
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();
    _tfClientId = tfClientId;
    _isHangulMode = TRUE;
    if (!_InitKeyEventSink() || !_InitLanguageBar())
    {
        Deactivate();
        return E_FAIL;
    }
    return S_OK;
}

// --- ITfCompositionSink Implementation ---
STDMETHODIMP CTextService::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition* pComposition)
{
    if (_pComposition)
    {
        _pComposition->Release();
        _pComposition = nullptr;
    }
    _krime.Reset(); // 조합 엔진 상태 초기화
    return S_OK;
}


// --- 내부 함수 구현 ---
BOOL CTextService::_InitKeyEventSink()
{
    ITfKeystrokeMgr* pKeystrokeMgr;
    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeystrokeMgr) != S_OK) return FALSE;
    HRESULT hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink*)this, TRUE);
    pKeystrokeMgr->Release();
    return (hr == S_OK);
}
void CTextService::_UninitKeyEventSink()
{
    ITfKeystrokeMgr* pKeystrokeMgr;
    if (_pThreadMgr && _pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeystrokeMgr) == S_OK)
    {
        pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);
        pKeystrokeMgr->Release();
    }
}

// --- 언어 입력기 초기화/해제 함수 구현 ---
BOOL CTextService::_InitLanguageBar()
{
    if (_pLangBar) return TRUE;

    ITfLangBarItemMgr* pLangBarItemMgr;
    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void**)&pLangBarItemMgr) != S_OK)
        return FALSE;

    _pLangBar = new CLanguageBar();
    if (_pLangBar == nullptr)
    {
        pLangBarItemMgr->Release();
        return FALSE;
    }

    pLangBarItemMgr->AddItem(_pLangBar);
    pLangBarItemMgr->Release();
    // _pLangBar의 소유권은 이제 LangBarMgr에 있으므로 여기서 Release하지 않습니다.

    return TRUE;
}

void CTextService::_UninitLanguageBar()
{
    if (_pLangBar)
    {
        ITfLangBarItemMgr* pLangBarItemMgr;
        if (_pThreadMgr && _pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void**)&pLangBarItemMgr) == S_OK)
        {
            pLangBarItemMgr->RemoveItem(_pLangBar);
            pLangBarItemMgr->Release();
        }
        _pLangBar->Release();
        _pLangBar = nullptr;
    }
}