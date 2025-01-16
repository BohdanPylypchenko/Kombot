module;

#include "pch.h"

export module winapi.consoleapi;

export import winapi.minwindef;
using Winapi::Minwindef::Dword;
using Winapi::Minwindef::Bool;

export namespace Winapi::Consoleapi
{
    enum class ControlEvent
    {
        C = CTRL_C_EVENT,
        Break = CTRL_BREAK_EVENT,
        Close = CTRL_CLOSE_EVENT,
        Logoff = CTRL_LOGOFF_EVENT,
        Shutdown = CTRL_SHUTDOWN_EVENT
    };

    using HandlerRoutine = PHANDLER_ROUTINE;

    inline Dword set_console_control_handler(
        HandlerRoutine handler_routine,
        Bool add
    ) {
        return SetConsoleCtrlHandler(handler_routine, add) == 0 ?
            GetLastError() : 0;
    }
}