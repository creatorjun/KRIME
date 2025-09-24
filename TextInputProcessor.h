//TextInputProcessor.h

#pragma once

#include <msctf.h>      // ITfTextInputProcessor, ITfKeyEventSink
#include "Globals.h"
#include "Log.h"

class CLangBarItem; // 전방 선언

class CTextInputProcessor : public ITfTextInputProcessor,
    public ITfKeyEventSink
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

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnPreservedKey(ITfContext* pic, REFGUID rguid, BOOL* pfEaten);

    // 언어 상태 변경을 위한 공개 함수
    void ToggleLanguage();

private:
    long _cRef;
    ITfThreadMgr* _pThreadMgr;
    TfClientId _clientId;

    CLangBarItem* _pLangBarItem;
    BOOL _isKoreanMode;
};