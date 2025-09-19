#include "pch.h"
#include "EditSession.h"
#include "TextService.h"

// --- CEditSessionBase ---
CEditSessionBase::CEditSessionBase(CTextService* pTextService, ITfContext* pContext)
{
    _pTextService = pTextService;
    _pTextService->AddRef();
    _pContext = pContext;
    if (_pContext) _pContext->AddRef();
    _refCount = 1;
}
CEditSessionBase::~CEditSessionBase()
{
    if (_pContext) _pContext->Release();
    if (_pTextService) _pTextService->Release();
}
STDMETHODIMP CEditSessionBase::QueryInterface(REFIID riid, void** ppvObj)
{
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfEditSession))
    {
        *ppvObj = (ITfEditSession*)this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}
STDMETHODIMP_(ULONG) CEditSessionBase::AddRef(void) { return ++_refCount; }
STDMETHODIMP_(ULONG) CEditSessionBase::Release(void)
{
    LONG cr = --_refCount;
    if (_refCount == 0) delete this;
    return cr;
}

// --- CInsertTextEditSession ---
CInsertTextEditSession::CInsertTextEditSession(CTextService* pTextService, ITfContext* pContext, const WCHAR* wcs)
    : CEditSessionBase(pTextService, pContext)
{
    _text = wcs;
}
STDMETHODIMP CInsertTextEditSession::DoEditSession(TfEditCookie ec)
{
    TF_SELECTION sel;
    ULONG cFetched;
    if (FAILED(_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &cFetched)) || cFetched == 0)
        return S_OK;

    // SetText 호출 수정: 4개의 인자 사용
    sel.range->SetText(ec, 0, _text.c_str(), (LONG)_text.length());
    sel.range->Collapse(ec, TF_ANCHOR_END);
    _pContext->SetSelection(ec, 1, &sel);

    sel.range->Release();
    return S_OK;
}

// --- CCompositionEditSession ---
CCompositionEditSession::CCompositionEditSession(CTextService* pTextService, ITfContext* pContext, const WCHAR* wcs)
    : CEditSessionBase(pTextService, pContext)
{
    _text = wcs;
}
STDMETHODIMP CCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    if (_text.empty())
    {
        _pTextService->_TerminateComposition(ec, _pContext);
    }
    else
    {
        _pTextService->_UpdateComposition(ec, _pContext, _text.c_str());
    }
    return S_OK;
}