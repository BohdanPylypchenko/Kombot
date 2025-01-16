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
//using Kombot::Frameshot::HdcScreen;
//using Kombot::Frameshot::FrameShooter;
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

        // alt is 164
        Config config
        {
            .aim_keycode = static_cast<int>('9'),
            .aim_config =
            {
                .target_color =
                {
                    .blue = 1,
                    .green = 2,
                    .red = 250
                },
                .frame_half_wh_px = 200,
                .screen_width_relation = 16,
                .screen_height_relation = 10,
                .mouse_x_ppd = 181,
                .mouse_y_ppd = 181,
                .horizontal_fov = 128,
            }
        };

        initialize(config);
        start();

        //HdcScreen screen { };
        //FrameShooter frameshooter
        //{
        //    screen,
        //    get_device_caps(screen.get(), DeviceCap::HorizontalResolution) / 2 - 200,
        //    get_device_caps(screen.get(), DeviceCap::VecticalResolution) / 2 - 200,
        //    400, 400
        //};
        //frameshooter.capture_frame_in_bitmap();
        //frameshooter.copy_bitmap_data_into_buffer();
        //frameshooter.dump_to_file("screenshot.bmp");
    }
}

int main(void)
{
    Playground::run();

    return 0;
}