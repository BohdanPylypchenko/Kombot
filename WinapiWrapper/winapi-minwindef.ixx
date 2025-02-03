module;

#include "pch.h"

export module winapi.minwindef;

export namespace Winapi::Minwindef
{
    using Byte = BYTE;
    using Word = WORD;
    using Dword = DWORD;
    using Long = LONG;
    using ULong = ULONG;
    using UInt = UINT;

    using Bool = BOOL;
    const Bool True = TRUE;
    const Bool False = FALSE;

    using WParam = WPARAM;
    using LParam = LPARAM;
    using LResult = LRESULT;
}
