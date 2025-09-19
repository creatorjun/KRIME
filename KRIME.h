#pragma once
#include "pch.h"

class CKrime
{
public:
    CKrime();

    void ProcessKey(wchar_t ch);
    void ProcessBackspace();
    void Reset();
    std::wstring GetCommitString();
    std::wstring GetPreeditString();

private:
    std::wstring _commitBuffer;
    std::wstring _preeditBuffer;
};