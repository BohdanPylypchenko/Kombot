module;

#include "pch.h"

export module winapi.winuser:input;

export import winapi.minwindef;
using Winapi::Minwindef::UInt;
using Winapi::Minwindef::Dword;

export namespace Winapi::WinUser::Input
{
    using Input = INPUT;
    using KeyboardInput = KEYBDINPUT;
    using MouseInput = MOUSEINPUT;

    enum class InputType : Dword
    {
        Mouse = INPUT_MOUSE,
        Keyboard = INPUT_KEYBOARD,
        Hardware = INPUT_HARDWARE
    };

    enum KeyboardEventFlag : Dword
    {
        ExtendKey = KEYEVENTF_EXTENDEDKEY,
        KeyUp = KEYEVENTF_KEYUP,
        ScanCode = KEYEVENTF_SCANCODE,
        Unicode = KEYEVENTF_UNICODE
    };

    enum MouseEventFlag : Dword
    {
        Move = MOUSEEVENTF_MOVE,
        LeftDown = MOUSEEVENTF_LEFTDOWN,
        LeftUp = MOUSEEVENTF_LEFTUP,
        RightDown = MOUSEEVENTF_RIGHTDOWN,
        RightUp = MOUSEEVENTF_RIGHTUP,
        MiddleDown = MOUSEEVENTF_MIDDLEDOWN,
        MiddleUp = MOUSEEVENTF_MIDDLEUP,
        XDown = MOUSEEVENTF_XDOWN,
        XUp = MOUSEEVENTF_XUP,
        VerticalWheel = MOUSEEVENTF_WHEEL,
        HorizontalWheel = MOUSEEVENTF_HWHEEL,
        MoveNoCoascale = MOUSEEVENTF_MOVE_NOCOALESCE,
        VirtualDesk = MOUSEEVENTF_VIRTUALDESK,
        Absolute = MOUSEEVENTF_ABSOLUTE
    };

    inline UInt send_input(UInt input_count, Input* inputs) {
        UInt result = SendInput(input_count, inputs, sizeof(INPUT));
        return result;
    }
}
