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
using namespace Kombot::Konfig;

import std;
using std::println, std::print;
using namespace std::chrono_literals;

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
            Kombot::cleanup();
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
            .on_off_keycode = static_cast<int>('9'),
            .aim_config =
            {
                .target_color =
                {
                    .blue = 10,
                    .green = 10,
                    .red = 245
                },
                .max_target_color_difference =
                {
                    //.blue = 25,
                    //.green = 25,
                    .blue = 10,
                    .green = 10,
                    .red = 10
                },
                .frame_half_wh_px = 128,
                .screen_width_relation = 16,
                .screen_height_relation = 10,
                .mouse_x_ppd = 1819,
                .mouse_y_ppd = 1819,
                //.mouse_x_ppd = 1136,
                //.mouse_y_ppd = 1136,
                .horizontal_fov = 128,

                // 57-60
                //.barrier_coefficient = 0.0,
                //.small_x_coefficient = 1,
                //.small_y_coefficient = 1

                // 60-63
                //.barrier_coefficient = 0.05,
                //.small_x_coefficient = 1,
                //.small_y_coefficient = 1
                
                // 60-63
                .barrier_coefficient = 0.1,
                .small_x_coefficient = 50,
                .small_y_coefficient = 50

                //.barrier_coefficient = 0.1,
                //.small_x_coefficient = 60,
                //.small_y_coefficient = 60
            },
            .keycodes_no = KeycodeSet { 'Q', '3' },
            .keycodes_always = KeycodeSet { '1', '2', '4', 'E' },
            .keycodes_on_target = KeycodeSet { 'R' },
            .mouse_trigger_type = MouseTriggerType::Left
        };

        print("\n\n===\n\n");
        println("{}", config.to_string());
        print("\n\n===\n\n");

        Kombot::run(config);
    }
}

int main(void)
{
    Playground::run();

    return 0;
}