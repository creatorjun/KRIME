#pragma once
#include "pch.h"

class CClassFactory : public IClassFactory
{
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj);
    STDMETHODIMP LockServer(BOOL fLock);

    CClassFactory();
    ~CClassFactory();

private:
    LONG _refCount;
};