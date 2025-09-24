#include "framework.h"
#include "TextInputProcessor.h"
#include "LangBar.h"

CTextInputProcessor::CTextInputProcessor()
{
    _cRef = 1;
    _pThreadMgr = nullptr;
    _clientId = TF_CLIENTID_NULL;
    _pLangBarItem = nullptr;
    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
    _isKoreanMode = TRUE;
    g_cRefDll++;
    LOG_WRITE("CTextInputProcessor object created.");
}

CTextInputProcessor::~CTextInputProcessor()
{
    g_cRefDll--;
    LOG_WRITE("CTextInputProcessor object destroyed.");
}

STDMETHODIMP CTextInputProcessor::QueryInterface(REFIID riid, void** ppvObj)
{
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor))
        *ppvObj = (ITfTextInputProcessor*)this;
    else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
        *ppvObj = (ITfThreadMgrEventSink*)this;
    else if (IsEqualIID(riid, IID_ITfKeyEventSink))
        *ppvObj = (ITfKeyEventSink*)this;

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CTextInputProcessor::AddRef(void) { return ++_cRef; }
STDMETHODIMP_(ULONG) CTextInputProcessor::Release(void)
{
    long cr = --_cRef;
    if (cr == 0) delete this;
    return cr;
}

STDMETHODIMP CTextInputProcessor::Activate(ITfThreadMgr* ptim, TfClientId tid)
{
    LOG_WRITE("Activate called. ClientId = %d", tid);
    _pThreadMgr = ptim;
    _pThreadMgr->AddRef();
    _clientId = tid;

    // 1. 스레드 관리자 이벤트 싱크 등록 (OnSetFocus를 받기 위함)
    ITfSource* pSource = nullptr;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource)))
    {
        pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink*)this, &_dwThreadMgrEventSinkCookie);
        pSource->Release();
    }

    // 2. 언어 표시줄 아이템 등록
    ITfLangBarItemMgr* pLangBarItemMgr = nullptr;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void**)&pLangBarItemMgr)))
    {
        _pLangBarItem = new CLangBarItem(this);
        if (_pLangBarItem)
        {
            pLangBarItemMgr->AddItem(_pLangBarItem);
            LOG_WRITE("Language bar item added.");
        }
        pLangBarItemMgr->Release();
    }
    return S_OK;
}

STDMETHODIMP CTextInputProcessor::Deactivate(void)
{
    LOG_WRITE("Deactivate called for ClientId = %d", _clientId);

    // 1. 스레드 관리자 이벤트 싱크 해제
    ITfSource* pSource = nullptr;
    if (_pThreadMgr && SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource)))
    {
        pSource->UnadviseSink(_dwThreadMgrEventSinkCookie);
        pSource->Release();
    }

    // 2. 언어 표시줄 아이템 제거
    ITfLangBarItemMgr* pLangBarItemMgr = nullptr;
    if (_pThreadMgr && SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void**)&pLangBarItemMgr)))
    {
        if (_pLangBarItem)
        {
            pLangBarItemMgr->RemoveItem(_pLangBarItem);
            _pLangBarItem->Release();
            _pLangBarItem = nullptr;
            LOG_WRITE("Language bar item removed.");
        }
        pLangBarItemMgr->Release();
    }

    if (_pThreadMgr)
    {
        _pThreadMgr->Release();
        _pThreadMgr = nullptr;
    }
    _clientId = TF_CLIENTID_NULL;
    LOG_WRITE("Deactivate successful.");
    return S_OK;
}


// --- ITfThreadMgrEventSink ---
STDMETHODIMP CTextInputProcessor::OnInitDocumentMgr(ITfDocumentMgr* pdim) { return S_OK; }
STDMETHODIMP CTextInputProcessor::OnUninitDocumentMgr(ITfDocumentMgr* pdim) { return S_OK; }

STDMETHODIMP CTextInputProcessor::OnSetFocus(ITfDocumentMgr* pdimFocus, ITfDocumentMgr* pdimPrevFocus)
{
    LOG_WRITE("OnSetFocus (ThreadMgrEventSink) called.");
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeystrokeMgr)))
        return S_OK;

    if (pdimFocus) // 포커스를 얻었을 때
    {
        LOG_WRITE("Gained focus, advising key event sink and preserving key.");
        pKeystrokeMgr->AdviseKeyEventSink(_clientId, (ITfKeyEventSink*)this, TRUE);

        TF_PRESERVEDKEY preservedKey;
        preservedKey.uVKey = VK_RMENU;
        preservedKey.uModifiers = 0;
        const WCHAR* desc = L"한/영 전환";
        pKeystrokeMgr->PreserveKey(_clientId, c_guidLangChangeKey, &preservedKey, desc, static_cast<ULONG>(wcslen(desc)));
    }
    else // 포커스를 잃었을 때
    {
        LOG_WRITE("Lost focus, unadvising key event sink and unpreserving key.");
        pKeystrokeMgr->UnpreserveKey(c_guidLangChangeKey, nullptr); // nullptr로 GUID에 해당하는 모든 키 해제
        pKeystrokeMgr->UnadviseKeyEventSink(_clientId);
    }
    pKeystrokeMgr->Release();
    return S_OK;
}
STDMETHODIMP CTextInputProcessor::OnPushContext(ITfContext* pic) { return S_OK; }
STDMETHODIMP CTextInputProcessor::OnPopContext(ITfContext* pic) { return S_OK; }


// --- ITfKeyEventSink ---
STDMETHODIMP CTextInputProcessor::OnSetFocus(BOOL fForeground) { return S_OK; }
STDMETHODIMP CTextInputProcessor::OnTestKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}
STDMETHODIMP CTextInputProcessor::OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}
STDMETHODIMP CTextInputProcessor::OnTestKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}
STDMETHODIMP CTextInputProcessor::OnKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}
STDMETHODIMP CTextInputProcessor::OnPreservedKey(ITfContext* pic, REFGUID rguid, BOOL* pfEaten)
{
    LOG_WRITE("OnPreservedKey called.");
    if (IsEqualGUID(rguid, c_guidLangChangeKey))
    {
        ToggleLanguage();
        *pfEaten = TRUE;
        return S_OK;
    }
    *pfEaten = FALSE;
    return S_OK;
}

// --- Public Methods ---
void CTextInputProcessor::ToggleLanguage()
{
    _isKoreanMode = !_isKoreanMode;
    LOG_WRITE("Language toggled to %s.", _isKoreanMode ? "Korean" : "English");
    if (_pLangBarItem)
    {
        _pLangBarItem->UpdateIcon(_isKoreanMode);
    }
}