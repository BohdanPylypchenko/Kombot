import winapi;
using Winapi::Minwindef::Bool;
using Winapi::Minwindef::Dword;
using Winapi::Minwindef::True;
using Winapi::Minwindef::False;
using Winapi::Consoleapi::ControlEvent;
using Winapi::Consoleapi::set_console_control_handler;
using Winapi::Display::get_device_caps;
using Winapi::Display::DeviceCap;

import kombot;
using Kombot::Config;
using Kombot::initialize;
using Kombot::start;
using Kombot::cleanup;

import std;
using std::println;

namespace Playground
{
    static Bool exit_handler(Dword control_event)
    {
        if (
            control_event == static_cast<Dword>(ControlEvent::C) ||
            control_event == static_cast<Dword>(ControlEvent::Close) ||
            control_event == static_cast<Dword>(ControlEvent::Logoff) ||
            control_event == static_cast<Dword>(ControlEvent::Shutdown)
        ) {
            cleanup();
        }
        return False;
    }

    static void run()
    {
        if (set_console_control_handler(exit_handler, True) != 0)
        {
            println("Failed to register playground destructor using atexit");
            return;
        }

        Config config
        {
            .aim_keycode = static_cast<int>('9'),
            .aim_config =
            {
                .target_color =
                {
                    .blue = 50,
                    .green = 50,
                    .red = 230
                },
                .max_target_color_difference =
                {
                    .blue = 25,
                    .green = 25,
                    //.blue = 10,
                    //.green = 10,
                    .red = 30
                },
                .frame_half_wh_px = 128,
                .screen_width_relation = 16,
                .screen_height_relation = 10,
                .mouse_x_ppd = 454,
                .mouse_y_ppd = 454,
                .horizontal_fov = 128,
                .barrier_coefficient = 0.9
            }
        };

        initialize(config);
        start();
    }
}

int main(void)
{
    Playground::run();

    return 0;
}