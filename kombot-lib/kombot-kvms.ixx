export module kombot:kvms;

import winapi;
using Winapi::Minwindef::Dword;
using Winapi::Minwindef::LResult;
using Winapi::Minwindef::WParam;
using Winapi::Minwindef::LParam;
using Winapi::WinUser::Msg::Hook;
using Winapi::WinUser::Msg::HookType;
using Winapi::WinUser::Key::KeyEvent;
using Winapi::WinUser::Mouse::MouseEvent;
using Winapi::WinUser::Msg::call_next_hook_ex;
using Winapi::WinUser::Msg::ActionHookCode;
using Winapi::WinUser::Key::KeyEventType;
using Winapi::WinUser::Key::KeyInfo;
using Winapi::WinUser::Mouse::MouseEventType;
using Winapi::WinUser::Mouse::MouseInfo;
using Winapi::WinUser::Msg::HookProc;

import :common;
using Kombot::Common::KeyEventHandler;
using Kombot::Common::MouseEventHandler;

import std;
using std::function;

export namespace Kombot::KVMS
{
    class KVMSHook
    {
    private:

        static KeyEventHandler key_event_handler;
        static MouseEventHandler mouse_event_handler;

    public:

        KVMSHook() = delete;

        static void initialize(
            KeyEventHandler key_event_handler, MouseEventHandler mouse_event_handler
        ) {
            KVMSHook::key_event_handler = key_event_handler;
            KVMSHook::mouse_event_handler = mouse_event_handler;
        }

        static LResult __stdcall key_hook_proc(int code, WParam wparam, LParam lparam)
        {
            if (key_event_handler && code == ActionHookCode)
            {
                KeyEvent kv =
                {
                    .type = static_cast<KeyEventType>(wparam),
                    .info = reinterpret_cast<KeyInfo*>(lparam)
                };
                key_event_handler(kv);
            }
            return call_next_hook_ex(code, wparam, lparam);
        }

        static LResult __stdcall mouse_hook_proc(int code, WParam wparam, LParam lparam)
        {
            if (mouse_event_handler && code == ActionHookCode)
            {
                MouseEvent ms =
                {
                    .type = static_cast<MouseEventType>(wparam),
                    .info = reinterpret_cast<MouseInfo*>(lparam)
                };
                if (!is_mouse_event_to_filter(ms)) mouse_event_handler(ms);
            }
            return call_next_hook_ex(code, wparam, lparam);
        }

    private:

        static inline bool is_mouse_event_to_filter(MouseEvent ms)
        {
            return
                ms.info->flags == static_cast<Dword>(MouseEventType::Move) ||
                ms.info->flags == static_cast<Dword>(MouseEventType::MiddleDown) ||
                ms.info->flags == static_cast<Dword>(MouseEventType::MiddleUp) ||
                ms.info->flags == static_cast<Dword>(MouseEventType::MiddleDouble);
        }
    };
    KeyEventHandler KVMSHook::key_event_handler = nullptr;
    MouseEventHandler KVMSHook::mouse_event_handler = nullptr;

    class InputListener
    {
    private:

        Hook key_hook;
        Hook mouse_hook;

    public:

        InputListener():
            key_hook(KVMSHook::key_hook_proc, HookType::Keyboard),
            mouse_hook(KVMSHook::mouse_hook_proc, HookType::Mouse)
        { }

        InputListener(HookProc kv_hook_proc):
            key_hook(kv_hook_proc, HookType::Keyboard),
            mouse_hook()
        { }

        InputListener(HookProc kv_hook_proc, HookProc ms_hook_proc):
            key_hook(kv_hook_proc, HookType::Keyboard),
            mouse_hook(ms_hook_proc, HookType::Mouse)
        { }

        InputListener(const InputListener& other) = delete;
        InputListener& operator=(const InputListener& other) = delete;

        InputListener(InputListener&& other) = default;
        InputListener& operator=(InputListener&& other) = default;
    };
}
