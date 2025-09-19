#include "pch.h"
#include "TextService.h"

// --- ITfKeyEventSink Implementation ---

STDMETHODIMP CTextService::OnSetFocus(BOOL fForeground) { return S_OK; }

STDMETHODIMP CTextService::OnTestKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
    // 한글 모드일 때는 모든 키를 가로챕니다. (예시)
    // TODO: 실제 한/영 상태를 관리하는 변수를 만들어 확인해야 합니다.
    *pfEaten = TRUE;
    return S_OK;
}

STDMETHODIMP CTextService::OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
    *pfEaten = TRUE; // 일단 모든 키 이벤트를 IME가 처리한다고 알림

    // 백스페이스 처리
    if (wParam == VK_BACK)
    {
        _krime.ProcessBackspace();
    }
    // 일반 문자키 처리 (A-Z)
    else if (wParam >= 'A' && wParam <= 'Z')
    {
        // TODO: Shift 키 상태 등을 고려하여 정확한 문자를 전달해야 합니다.
        _krime.ProcessKey((wchar_t)wParam);
    }
    // 스페이스바 처리
    else if (wParam == VK_SPACE)
    {
        _krime.ProcessKey(L' ');
    }
    else
    {
        // 처리하지 않는 키는 시스템에 넘김
        *pfEaten = FALSE;
    }


    // 1. 완성된 글자가 있는지 확인하고 삽입
    std::wstring commitString = _krime.GetCommitString();
    if (!commitString.empty())
    {
        _InsertText(pContext, commitString.c_str());
    }

    // 2. 조합 중인 글자가 있는지 확인하고 화면에 표시
    std::wstring preeditString = _krime.GetPreeditString();
    _UpdateCompositionDisplay(pContext, preeditString.c_str());

    return S_OK;
}

STDMETHODIMP CTextService::OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
    *pfEaten = TRUE;
    return S_OK;
}

STDMETHODIMP CTextService::OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
{
    *pfEaten = TRUE;
    return S_OK;
}

STDMETHODIMP CTextService::OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}