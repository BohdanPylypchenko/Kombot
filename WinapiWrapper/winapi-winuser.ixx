module;

#include "pch.h"

export module winapi.winuser;

export import winapi.minwindef;
export import :msg;
export import :key;
export import :mouse;
export import :input;

export namespace Winapi::WinUser
{
    using namespace Winapi::WinUser::Msg;
    using namespace Winapi::WinUser::Key;
    using namespace Winapi::WinUser::Mouse;
    using namespace Winapi::WinUser::Input;
}
