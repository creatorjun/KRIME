#include "pch.h"
#include "TextService.h"

// --- Constructor / Destructor ---
CTextService::CTextService()
{
    _refCount = 1;
    _pThreadMgr = nullptr;
    _tfClientId = TF_CLIENTID_NULL;
}

CTextService::~CTextService()
{
}

// --- Static Factory Method ---
HRESULT CTextService::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj)
{
    if (ppvObj == nullptr)
        return E_INVALIDARG;
    *ppvObj = nullptr;

    if (pUnkOuter != nullptr)
        return CLASS_E_NOAGGREGATION;

    CTextService* pTextService = new CTextService();
    if (pTextService == nullptr)
        return E_OUTOFMEMORY;

    HRESULT hr = pTextService->QueryInterface(riid, ppvObj);
    pTextService->Release(); // QueryInterface will increase ref count
    return hr;
}

// --- IUnknown Implementation ---
STDMETHODIMP CTextService::QueryInterface(REFIID riid, void** ppvObj)
{
    if (ppvObj == nullptr)
        return E_INVALIDARG;
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextInputProcessor))
    {
        *ppvObj = (ITfTextInputProcessor*)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx))
    {
        *ppvObj = (ITfTextInputProcessorEx*)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CTextService::AddRef(void)
{
    return ++_refCount;
}

STDMETHODIMP_(ULONG) CTextService::Release(void)
{
    LONG cr = --_refCount;
    if (_refCount == 0)
    {
        delete this;
    }
    return cr;
}

// --- ITfTextInputProcessor / ITfTextInputProcessorEx Implementation ---
STDMETHODIMP CTextService::Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId)
{
    // ITfTextInputProcessorEx의 ActivateEx를 호출합니다.
    return this->ActivateEx(pThreadMgr, tfClientId, 0);
}

STDMETHODIMP CTextService::Deactivate()
{
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

    // TODO: 여기에 IME 활성화 시 필요한 초기화 코드를 추가합니다.
    // (예: 키보드 이벤트 싱크 등록, 언어바 아이콘 초기화 등)

    return S_OK;
}