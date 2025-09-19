#pragma once
#include "pch.h"
#include "KRIME.h"

// CTextService가 CLanguageBar의 포인터만 알면 되므로,
// 전체 헤더를 포함하는 대신 전방 선언합니다.
class CLanguageBar;

class CTextService : public ITfTextInputProcessorEx,
    public ITfKeyEventSink,
    public ITfCompositionSink
{
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfTextInputProcessor / ITfTextInputProcessorEx
    STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId);
    STDMETHODIMP Deactivate();
    STDMETHODIMP ActivateEx(ITfThreadMgr* pThreadMgr, TfClientId tfClientId, DWORD dwFlags);

    // ITfKeyEventSink
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

    // Private Helper Functions
    BOOL _InitKeyEventSink();
    void _UninitKeyEventSink();
    BOOL _InitLanguageBar();
    void _UninitLanguageBar();
    void _InsertText(ITfContext* pContext, const WCHAR* wcs);
    void _UpdateCompositionDisplay(ITfContext* pContext, const WCHAR* wcs);
    BOOL _IsComposing();

    LONG _refCount;
    ITfThreadMgr* _pThreadMgr;
    TfClientId _tfClientId;

    CKrime _krime;
    ITfComposition* _pComposition;
    CLanguageBar* _pLangBar;
    BOOL _isHangulMode;
};