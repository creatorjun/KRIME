#include "ClassFactory.h"
#include "TextInputProcessor.h"
#include "Globals.h"

CClassFactory::CClassFactory()
{
    _cRef = 1;
    g_cRefDll++;
}

CClassFactory::~CClassFactory()
{
    g_cRefDll--;
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppvObj)
{
    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppvObj = (IClassFactory*)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef(void)
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CClassFactory::Release(void)
{
    long cr = --_cRef;
    if (cr == 0)
    {
        delete this;
    }
    return cr;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj)
{
    if (pUnkOuter != nullptr)
    {
        return CLASS_E_NOAGGREGATION;
    }

    CTextInputProcessor* pTIP = new CTextInputProcessor();
    if (pTIP == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pTIP->QueryInterface(riid, ppvObj);
    pTIP->Release(); // QueryInterface가 AddRef를 하므로 여기서 Release
    return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        g_cRefDll++;
    }
    else
    {
        g_cRefDll--;
    }
    return S_OK;
}