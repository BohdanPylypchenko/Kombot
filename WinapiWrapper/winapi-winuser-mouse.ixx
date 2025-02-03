module;

#include "pch.h"

export module winapi.winuser:mouse;

export namespace Winapi::WinUser::Mouse
{
    enum class MouseEventType
    {
        First = WM_MOUSEFIRST,
        Move = WM_MOUSEMOVE,
        LeftDown = WM_LBUTTONDOWN,
        LeftUp = WM_LBUTTONUP,
        LeftDouble = WM_LBUTTONDBLCLK,
        RightDown = WM_RBUTTONDOWN,
        RightUp = WM_RBUTTONUP,
        RightDouble = WM_RBUTTONDBLCLK,
        MiddleDown = WM_MBUTTONDOWN,
        MiddleUp = WM_MBUTTONUP,
        MiddleDouble = WM_MBUTTONDBLCLK,
        Wheel = WM_MOUSEWHEEL,
        XDown = WM_XBUTTONDOWN,
        XUp =  WM_XBUTTONUP,
        XDouble = WM_XBUTTONDBLCLK,
        Last = WM_MOUSELAST,
    };
    
    using MouseInfo = MSLLHOOKSTRUCT;
    
    struct MouseEvent
    {
        MouseEventType type;
        MouseInfo* info;
    };
}
