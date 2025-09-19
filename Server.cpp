#include "pch.h"
#include "Server.h"
#include "TextService.h"
#include "Global.h"

// --- ClassFactory Implementation ---
CClassFactory::CClassFactory()
{
    _refCount = 1;
}

CClassFactory::~CClassFactory()
{
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppvObj)
{
    if (ppvObj == nullptr)
        return E_INVALIDARG;
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef(void)
{
    return ++_refCount;
}

STDMETHODIMP_(ULONG) CClassFactory::Release(void)
{
    LONG cr = --_refCount;
    if (cr == 0)
    {
        delete this;
    }
    return cr;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj)
{
    // CTextService의 인스턴스를 생성합니다.
    return CTextService::CreateInstance(pUnkOuter, riid, ppvObj);
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        InterlockedIncrement(&g_dllRefCount);
    }
    else
    {
        InterlockedDecrement(&g_dllRefCount);
    }
    return S_OK;
}


// --- DLL Export Functions (SAL 주석 추가) ---
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
    if (!IsEqualCLSID(rclsid, CLSID_KRIME))
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    CClassFactory* pFactory = new CClassFactory();
    if (pFactory == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    return (g_dllRefCount >= 0) ? S_FALSE : S_OK;
}