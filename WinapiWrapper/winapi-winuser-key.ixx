module;

#include "pch.h"

export module winapi.winuser:key;

export namespace Winapi::WinUser::Key
{
    enum class KeyEventType
    {
        First = WM_KEYFIRST,
        Down = WM_KEYDOWN,
        Up = WM_KEYUP,
        Char = WM_CHAR,
        DeadChar = WM_DEADCHAR,
        SysDown = WM_SYSKEYDOWN,
        SysUp = WM_SYSKEYUP,
        SysChar = WM_SYSCHAR,
        SysDeadChar = WM_SYSDEADCHAR,
        Last = WM_KEYLAST
    };
    
    using KeyInfo = KBDLLHOOKSTRUCT;
    
    struct KeyEvent
    {
        KeyEventType type;
        KeyInfo* info;
    };
}