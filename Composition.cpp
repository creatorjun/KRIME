#include "pch.h"
#include "TextService.h"
#include "EditSession.h"

void CTextService::_InsertText(ITfContext* pContext, const WCHAR* wcs)
{
    CInsertTextEditSession* pEditSession = new CInsertTextEditSession(this, pContext, wcs);
    HRESULT hr;
    pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
    if (pEditSession) pEditSession->Release();
}

void CTextService::_UpdateCompositionDisplay(ITfContext* pContext, const WCHAR* wcs)
{
    CCompositionEditSession* pEditSession = new CCompositionEditSession(this, pContext, wcs);
    HRESULT hr;
    pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
    if (pEditSession) pEditSession->Release();
}

BOOL CTextService::_IsComposing()
{
    return (_pComposition != nullptr);
}

void CTextService::_UpdateComposition(TfEditCookie ec, ITfContext* pContext, const WCHAR* wcs)
{
    if (!_IsComposing())
    {
        ITfContextComposition* pContextComposition;
        if (SUCCEEDED(pContext->QueryInterface(IID_ITfContextComposition, (void**)&pContextComposition)))
        {
            TF_SELECTION sel;
            ULONG cFetched;
            if (SUCCEEDED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &cFetched)) && cFetched > 0)
            {
                pContextComposition->StartComposition(ec, sel.range, (ITfCompositionSink*)this, &_pComposition);
                sel.range->Release();
            }
            pContextComposition->Release();
        }
    }

    if (_pComposition)
    {
        ITfRange* pRange;
        if (SUCCEEDED(_pComposition->GetRange(&pRange)))
        {
            // SetText 호출 수정: 4개의 인자 사용
            pRange->SetText(ec, 0, wcs, (LONG)wcslen(wcs));
            pRange->Release();
        }
    }
}

void CTextService::_TerminateComposition(TfEditCookie ec, ITfContext* pContext)
{
    if (_pComposition)
    {
        _pComposition->EndComposition(ec);
    }
}