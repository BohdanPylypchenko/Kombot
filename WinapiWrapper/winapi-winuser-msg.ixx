module;

#include "pch.h"

export module winapi.winuser:msg;

export import winapi.minwindef;
using Winapi::Minwindef::LResult;
using Winapi::Minwindef::WParam;
using Winapi::Minwindef::LParam;

import std;
using std::format;
using std::error_code, std::system_error, std::system_category;

export namespace Winapi::WinUser::Msg
{
    using HookHandle = HHOOK;
    using HookProc = HOOKPROC;
    constexpr int ActionHookCode = HC_ACTION;

    void start_message_loop()
    {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    inline LResult call_next_hook_ex(
        HookHandle hook_handle, int code, WParam wparam, LParam lparam
    ) {
        return CallNextHookEx(hook_handle, code, wparam, lparam);
    }

    inline LResult call_next_hook_ex(
        int code, WParam wparam, LParam lparam
    ) {
        return CallNextHookEx(NULL, code, wparam, lparam);
    }

    enum class HookType
    {
        Keyboard = WH_KEYBOARD_LL,
        Mouse = WH_MOUSE_LL
    };

    class Hook
    {
    private:

        HHOOK hook;
        HookType type;

    public:

        Hook(): hook(), type()
        { }

        Hook(HookProc hook_proc, HookType type):
            hook(SetWindowsHookEx(static_cast<int>(type), hook_proc, NULL, 0)),
            type(type)
        {
            if (!hook)
            {
                throw system_error(
                    error_code(GetLastError(), std::system_category()),
                    format("Error: failed to create hook of type {}", static_cast<int>(type))
                );
            }
        }

        Hook(const Hook& other) = delete;
        Hook& operator=(const Hook& other) = delete;

        Hook(Hook&& other):
            hook(other.hook),
            type(other.type)
        {
            other.hook = nullptr;
        }

        Hook& operator=(Hook&& other)
        {
            hook = other.hook;
            type = other.type;
            other.hook = nullptr;
            return *this;
        }

        ~Hook()
        {
            if (hook) UnhookWindowsHookEx(hook);
        }
    };
}
