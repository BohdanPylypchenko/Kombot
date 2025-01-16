module;

#include "pch.h"

export module winapi.winuser:input;

export import winapi.minwindef;
using Winapi::Minwindef::UInt;

export namespace Winapi::WinUser::Input
{
    using Input = INPUT;
    using MouseInput = MOUSEINPUT;

    constexpr int InputMouseType = INPUT_MOUSE;
    constexpr int MouseEventMove = MOUSEEVENTF_MOVE;

    inline UInt send_input(UInt input_count, Input* inputs) {
        UInt result = SendInput(input_count, inputs, sizeof(INPUT));
        return result;
    }
}
