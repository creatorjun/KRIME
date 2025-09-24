#pragma once

#include <msctf.h>
#include "Globals.h"
#include "Log.h"

class CLangBarItem;

class CTextInputProcessor : public ITfTextInputProcessor,
    public ITfThreadMgrEventSink, // ������ ������ �̺�Ʈ�� �ޱ� ���� �������̽�
    public ITfKeyEventSink     // Ű �̺�Ʈ�� �ޱ� ���� �������̽�
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
    DWORD _dwThreadMgrEventSinkCookie; // ������ ������ �̺�Ʈ ��ũ ��Ű
    BOOL _isKoreanMode;
};