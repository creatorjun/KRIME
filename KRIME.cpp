#include "pch.h"
#include "KRIME.h"

CKrime::CKrime()
{
}

void CKrime::ProcessKey(wchar_t ch)
{
    if (ch == L' ')
    {
        if (!_preeditBuffer.empty())
        {
            _commitBuffer = _preeditBuffer;
            _preeditBuffer.clear();
        }
        _commitBuffer += L' ';
    }
    else
    {
        _preeditBuffer += ch;
    }
}

void CKrime::ProcessBackspace()
{
    if (!_preeditBuffer.empty())
    {
        _preeditBuffer.pop_back();
    }
}

void CKrime::Reset()
{
    _commitBuffer.clear();
    _preeditBuffer.clear();
}

std::wstring CKrime::GetCommitString()
{
    std::wstring temp = _commitBuffer;
    _commitBuffer.clear();
    return temp;
}

std::wstring CKrime::GetPreeditString()
{
    return _preeditBuffer;
}