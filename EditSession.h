#pragma once
#include "pch.h"

class CTextService;

class CEditSessionBase : public ITfEditSession
{
public:
    CEditSessionBase(CTextService* pTextService, ITfContext* pContext);
    virtual ~CEditSessionBase();

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) = 0;

protected:
    CTextService* _pTextService;
    ITfContext* _pContext;
    LONG _refCount;
};

class CInsertTextEditSession : public CEditSessionBase
{
public:
    CInsertTextEditSession(CTextService* pTextService, ITfContext* pContext, const WCHAR* wcs);
    STDMETHODIMP DoEditSession(TfEditCookie ec);
private:
    std::wstring _text;
};

class CCompositionEditSession : public CEditSessionBase
{
public:
    CCompositionEditSession(CTextService* pTextService, ITfContext* pContext, const WCHAR* wcs);
    STDMETHODIMP DoEditSession(TfEditCookie ec);
private:
    std::wstring _text;
};