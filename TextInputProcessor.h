#pragma once

#include <msctf.h>
#include "Globals.h"
#include "Log.h"

class CLangBarItem;

class CTextInputProcessor : public ITfTextInputProcessor,
    public ITfThreadMgrEventSink, // 스레드 관리자 이벤트를 받기 위한 인터페이스
    public ITfKeyEventSink     // 키 이벤트를 받기 위한 인터페이스
{
public:
    CTextInputProcessor();
    virtual ~CTextInputProcessor();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr* ptim, TfClientId tid);
    STDMETHODIMP Deactivate(void);

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr* pdim);
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr* pdim);
    STDMETHODIMP OnSetFocus(ITfDocumentMgr* pdimFocus, ITfDocumentMgr* pdimPrevFocus);
    STDMETHODIMP OnPushContext(ITfContext* pic);
    STDMETHODIMP OnPopContext(ITfContext* pic);

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnPreservedKey(ITfContext* pic, REFGUID rguid, BOOL* pfEaten);

    void ToggleLanguage();

private:
    long _cRef;
    ITfThreadMgr* _pThreadMgr;
    TfClientId _clientId;

    CLangBarItem* _pLangBarItem;
    DWORD _dwThreadMgrEventSinkCookie; // 스레드 관리자 이벤트 싱크 쿠키
    BOOL _isKoreanMode;
};