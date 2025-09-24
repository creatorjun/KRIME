#pragma once

#include <ctffunc.h>
#include "resource.h"

// Forward declaration
class CTextInputProcessor;

class CLangBarItem : public ITfLangBarItemButton,
    public ITfSource
{
public:
    CLangBarItem(CTextInputProcessor* pTIP);
    ~CLangBarItem();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfLangBarItem
    STDMETHODIMP GetInfo(TF_LANGBARITEMINFO* pInfo);
    STDMETHODIMP GetStatus(DWORD* pdwStatus);
    STDMETHODIMP Show(BOOL fShow);
    STDMETHODIMP GetTooltipString(BSTR* pbstrToolTip);

    // ITfLangBarItemButton
    STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT* prcArea);
    STDMETHODIMP InitMenu(ITfMenu* pMenu);
    STDMETHODIMP OnMenuSelect(UINT wID);
    STDMETHODIMP GetIcon(HICON* phIcon);
    STDMETHODIMP GetText(BSTR* pbstrText);

    // ITfSource
    STDMETHODIMP AdviseSink(REFIID riid, IUnknown* punk, DWORD* pdwCookie);
    STDMETHODIMP UnadviseSink(DWORD dwCookie);

    void UpdateIcon(BOOL isKorean);

private:
    long _cRef;
    CTextInputProcessor* _pTIP; // TIP의 포인터
    ITfLangBarItemSink* _pLangBarItemSink;
    DWORD _dwSinkCookie;

    HICON _hIconMain;
    HICON _hIconKor;
    HICON _hIconEng;
    BOOL _isKoreanMode;
};