#pragma once
#include "pch.h"

class CTextService : public ITfTextInputProcessorEx
{
public:
    // --- IUnknown ---
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // --- ITfTextInputProcessor / ITfTextInputProcessorEx ---
    STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId);
    STDMETHODIMP Deactivate();
    STDMETHODIMP ActivateEx(ITfThreadMgr* pThreadMgr, TfClientId tfClientId, DWORD dwFlags);

    // --- Static Factory Method ---
    static HRESULT CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj);

private:
    CTextService();
    ~CTextService();

    LONG _refCount;
    ITfThreadMgr* _pThreadMgr;
    TfClientId _tfClientId;
};