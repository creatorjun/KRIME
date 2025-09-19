#pragma once
#include "pch.h"
#include "KRIME.h" // CKrime 클래스의 전체 정의를 위해 포함

class CTextService : public ITfTextInputProcessorEx,
    public ITfKeyEventSink,
    public ITfCompositionSink
{
public:
    // IUnknown, ITfTextInputProcessorEx, ITfKeyEventSink (이전과 동일)
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId);
    STDMETHODIMP Deactivate();
    STDMETHODIMP ActivateEx(ITfThreadMgr* pThreadMgr, TfClientId tfClientId, DWORD dwFlags);
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pfEaten);

    // ITfCompositionSink
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition* pComposition);

    static HRESULT CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj);

    // Public methods for Edit Sessions
    void _UpdateComposition(TfEditCookie ec, ITfContext* pContext, const WCHAR* wcs);
    void _TerminateComposition(TfEditCookie ec, ITfContext* pContext);

private:
    CTextService();
    ~CTextService();

    BOOL _InitKeyEventSink();
    void _UninitKeyEventSink();

    // 내부 텍스트 처리 헬퍼 함수
    void _InsertText(ITfContext* pContext, const WCHAR* wcs);
    void _UpdateCompositionDisplay(ITfContext* pContext, const WCHAR* wcs);
    BOOL _IsComposing();

    LONG _refCount;
    ITfThreadMgr* _pThreadMgr;
    TfClientId _tfClientId;

    CKrime _krime;
    ITfComposition* _pComposition;
};