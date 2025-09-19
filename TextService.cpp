#include "pch.h"
#include "TextService.h"

// --- Constructor / Destructor ---
CTextService::CTextService()
{
    _refCount = 1;
    _pThreadMgr = nullptr;
    _tfClientId = TF_CLIENTID_NULL;
    _pComposition = nullptr; // _pComposition 초기화 추가
}

CTextService::~CTextService()
{
}

// ... CreateInstance, IUnknown, Activate/Deactivate, _Init/_UninitKeyEventSink ...
// (이전 단계에서 제공한 코드와 동일)

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
    if (!_InitKeyEventSink())
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