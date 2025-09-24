// TextInputProcessor.cpp

#include "framework.h"
#include "TextInputProcessor.h"
#include "LangBar.h"

CTextInputProcessor::CTextInputProcessor()
{
    _cRef = 1;
    _pThreadMgr = nullptr;
    _clientId = TF_CLIENTID_NULL;
    _pLangBarItem = nullptr;
    _isKoreanMode = TRUE; // 기본 상태는 한글
    g_cRefDll++;
    LOG_WRITE("CTextInputProcessor object created.");
}

CTextInputProcessor::~CTextInputProcessor()
{
    if (_pLangBarItem)
    {
        _pLangBarItem->Release();
    }
    if (_pThreadMgr)
    {
        _pThreadMgr->Release();
    }
    g_cRefDll--;
    LOG_WRITE("CTextInputProcessor object destroyed.");
}

STDMETHODIMP CTextInputProcessor::QueryInterface(REFIID riid, void** ppvObj)
{
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor))
        *ppvObj = (ITfTextInputProcessor*)this;
    else if (IsEqualIID(riid, IID_ITfKeyEventSink))
        *ppvObj = (ITfKeyEventSink*)this;

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CTextInputProcessor::AddRef(void)
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CTextInputProcessor::Release(void)
{
    long cr = --_cRef;
    if (cr == 0)
    {
        delete this;
    }
    return cr;
}

STDMETHODIMP CTextInputProcessor::Activate(ITfThreadMgr* ptim, TfClientId tid)
{
    LOG_WRITE("Activate started. ClientId = %d", tid);
    _pThreadMgr = ptim;
    _pThreadMgr->AddRef();
    _clientId = tid;

    // 1. 키스트로크 관리자(Keystroke Manager)를 얻어와서 키 이벤트를 받을 준비를 합니다.
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeystrokeMgr)))
    {
        // AdviseKeyEventSink: 이 클래스(this)가 키보드 이벤트를 받겠다고 TSF에 알립니다.
        pKeystrokeMgr->AdviseKeyEventSink(_clientId, (ITfKeyEventSink*)this, TRUE);
        LOG_WRITE("Key event sink advised.");

        // PreserveKey: 오른쪽 Alt 키를 시스템에서 사용하지 않고 우리 IME가 직접 처리하겠다고 예약합니다.
        TF_PRESERVEDKEY preservedKey;
        preservedKey.uVKey = VK_RMENU;
        preservedKey.uModifiers = 0;
        const WCHAR* desc = L"한/영 전환";
        pKeystrokeMgr->PreserveKey(_clientId, c_guidLangChangeKey, &preservedKey, desc, static_cast<ULONG>(wcslen(desc)));
        LOG_WRITE("Right Alt key preserved.");

        pKeystrokeMgr->Release();
    }
    else
    {
        LOG_WRITE("Failed to get ITfKeystrokeMgr.");
    }


    // 2. 언어 표시줄(Language Bar) 관리자를 얻어와서 아이콘을 등록합니다.
    ITfLangBarItemMgr* pLangBarItemMgr = nullptr;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void**)&pLangBarItemMgr)))
    {
        if (_pLangBarItem == nullptr)
        {
            _pLangBarItem = new CLangBarItem(this);
        }

        if (_pLangBarItem)
        {
            pLangBarItemMgr->AddItem(_pLangBarItem);
            LOG_WRITE("Language bar item added.");
        }
        pLangBarItemMgr->Release();
    }
    else
    {
        LOG_WRITE("Failed to get ITfLangBarItemMgr.");
    }

    LOG_WRITE("Activate finished.");
    return S_OK;
}

STDMETHODIMP CTextInputProcessor::Deactivate(void)
{
    LOG_WRITE("Deactivate started for ClientId = %d", _clientId);

    if (_pThreadMgr)
    {
        // 1. 언어 표시줄 아이템을 제거합니다.
        ITfLangBarItemMgr* pLangBarItemMgr = nullptr;
        if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void**)&pLangBarItemMgr)))
        {
            if (_pLangBarItem)
            {
                pLangBarItemMgr->RemoveItem(_pLangBarItem);
                LOG_WRITE("Language bar item removed.");
            }
            pLangBarItemMgr->Release();
        }

        // 2. 키 이벤트 싱크와 보존키 등록을 해제합니다.
        ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
        if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeystrokeMgr)))
        {
            TF_PRESERVEDKEY preservedKey;
            preservedKey.uVKey = VK_RMENU;
            preservedKey.uModifiers = 0;
            pKeystrokeMgr->UnpreserveKey(c_guidLangChangeKey, &preservedKey);
            pKeystrokeMgr->UnadviseKeyEventSink(_clientId);
            LOG_WRITE("Key event sink unadvised and key unpreserved.");
            pKeystrokeMgr->Release();
        }

        _pThreadMgr->Release();
        _pThreadMgr = nullptr;
    }

    _clientId = TF_CLIENTID_NULL;
    LOG_WRITE("Deactivate finished.");
    return S_OK;
}

// ITfKeyEventSink 메서드 구현
STDMETHODIMP CTextInputProcessor::OnSetFocus(BOOL fForeground)
{
    LOG_WRITE("OnSetFocus (KeyEventSink) called with fForeground = %s", fForeground ? "TRUE" : "FALSE");
    return S_OK;
}

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

// Public 메서드 구현
void CTextInputProcessor::ToggleLanguage()
{
    _isKoreanMode = !_isKoreanMode;
    LOG_WRITE("Language toggled to %s.", _isKoreanMode ? "Korean" : "English");
    if (_pLangBarItem)
    {
        _pLangBarItem->UpdateIcon(_isKoreanMode);
    }
}