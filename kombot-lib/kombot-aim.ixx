export module kombot:aim;

import winapi;
using Winapi::Minwindef::Byte;
using Winapi::Minwindef::Dword;
using Winapi::WinUser::Input::Input;
using Winapi::WinUser::Input::InputMouseType;
using Winapi::WinUser::Input::MouseEventMove;
using Winapi::WinUser::Input::send_input;
using Winapi::Display::BgrPixel;
using Winapi::Display::get_device_caps;
using Winapi::Display::DeviceCap;

import :common;
using Kombot::Common::State;

import :frameshot;
using Kombot::Frameshot::HdcScreen;
using Kombot::Frameshot::FrameShooter;

import std;
using std::atomic_flag;
using std::thread;
using std::fabs;
using std::invalid_argument;
using std::vector;
using std::println;
using std::cerr;
using std::string;
using std::shared_ptr;
using std::chrono::system_clock;

export namespace Kombot::Aim
{
    struct AimConfig
    {
        BgrPixel target_color;
        int frame_half_wh_px;
        int screen_width_relation;
        int screen_height_relation;
        int mouse_x_ppd;
        int mouse_y_ppd;
        int horizontal_fov;
    };

    class Aimer
    {
    private:

        struct Point
        {
            int x;
            int y;
            int m2;

            Point(): x(0), y(0), m2(0)
            { }

            Point(int x, int y):
                x(x), y(y),
                m2(x * x + y * y)
            { }

            Point(const Point& other) = default;
            Point& operator=(const Point& other) = default;

            Point(Point&& other) = default;
            Point& operator=(Point&& other) = default;

            bool operator==(const Point& other) const
            {
                return x == other.x && y == other.y && m2 == other.m2;
            }
        };

        atomic_flag is_kombot_running;
        atomic_flag is_aiming;
        State& state;

        int screen_resolution_w;
        int screen_resolution_h;
        int half_frame_wh;
        int frame_wh;
        int screen_pixels_per_degree;

        BgrPixel target_color;
        vector<Point> target;

        Point previous_target_average;

        FrameShooter frame_shooter;

    public:

        Aimer(HdcScreen& screen, State& state, AimConfig config):
            is_kombot_running(),
            is_aiming(),
            state(state),
            screen_resolution_w(get_device_caps(screen.get(), DeviceCap::HorizontalResolution)),
            screen_resolution_h(get_device_caps(screen.get(), DeviceCap::VecticalResolution)),
            half_frame_wh(config.frame_half_wh_px),
            frame_wh(half_frame_wh * 2),
            target_color(config.target_color),
            target { },
            previous_target_average(config.frame_half_wh_px, config.frame_half_wh_px),
            frame_shooter
            {
                screen,
                screen_resolution_w / 2 - half_frame_wh,
                screen_resolution_h / 2 - half_frame_wh,
                frame_wh,
                frame_wh
            }
        {
            is_kombot_running.test_and_set();
            is_aiming.test_and_set();

            double t1 =
                (double)(screen_resolution_w) /
                (double)(config.horizontal_fov);
            double aim_vertical_fow =
                (double)(config.horizontal_fov) /
                (double)(config.screen_width_relation) *
                (double)(config.screen_height_relation);
            double t2 = (double)screen_resolution_h /
                                aim_vertical_fow;
            if (fabs(t1 - t2) > 0.001)
            {
                throw invalid_argument
                {
                    "Error: failed screen pixels per degree relation check."
                };
            }
            screen_pixels_per_degree = static_cast<int>(t1);

            target.reserve(frame_wh * frame_wh);
        }

        Aimer(const Aimer& other) = delete;
        Aimer& operator=(const Aimer& other) = delete;

        Aimer(Aimer&& other) = default;
        Aimer& operator=(Aimer&& other) = default;

        void run()
        {
            thread t
            {
                [this]()
                {
                    while (is_kombot_running.test())
                    {
                        if (check_key_trigger() || check_mouse_trigger())
                        {
                            target.clear();
                            dummy_aim_iteration();
                        }
                    }
                    is_aiming.clear();
                }
            };
            t.detach();
        }

        inline void notify_on_kombot_end()
        {
            is_kombot_running.clear();
            for (auto i = 0; is_aiming.test(); i++);
        }

    private:

        inline void dummy_aim_iteration()
        {
            if (!frame_shooter.capture_frame_in_bitmap())
            {
                println(cerr, "{}", "error: failed to capture frame");
                return;
            }

            if (!frame_shooter.copy_bitmap_data_into_buffer())
            {
                println(cerr, "{}", "error: failed to copy bitmap data into buffer");
                return;
            }

            detect_target();
            if (target.empty())
            {
                println("target is empty");
                return;
            }

            println("target size = {}", target.size());

            Point current_target_average = target_average();
            if (previous_target_average == current_target_average)
                return;

            aim_on_point(convert_to_aim_point(current_target_average));

            previous_target_average = current_target_average;
        }

        inline void detect_target()
        {
            for (int y = 0; y < frame_wh; y++)
            {
                int y_index_offset = y * frame_wh;
                for (int x = 0; x < frame_wh; x++)
                {
                    int index = y_index_offset + x;
                    BgrPixel* const current = &(reinterpret_cast<BgrPixel*>(frame_shooter.get_buffer())[index]);

                    if (current->blue == target_color.blue &&
                        current->green == target_color.green &&
                        current->red == target_color.red)
                    {
                        target.push_back(Point(x, y));
                    }
                    else
                    {
                        current->blue = 40;
                        current->green = 200;
                        current->red = 40;
                    }
                }
            }
        }

        inline Point target_average() const
        {
            int sum_x = 0;
            int sum_y = 0;

            for (auto& p : target)
            {
                sum_x += p.x;
                sum_y += p.y;
            }

            return Point(
                sum_x / static_cast<int>(target.size()),
                sum_y / static_cast<int>(target.size())
            );
        }

        inline Point convert_to_aim_point(const Point point) const
        {
            return Point(point.x - half_frame_wh, point.y - half_frame_wh);
        }

        static inline void aim_on_point(const Point point)
        {
            Input input { };
            input.type = InputMouseType;
            input.mi.dwFlags = MouseEventMove;
            input.mi.dx = point.x * 10;
            input.mi.dy = point.y * 10;
            println("send input: x = {}, y = {}", input.mi.dx, input.mi.dy);
            send_input(1, &input);
        }

        inline bool check_key_trigger() const
        {
            return state.get<shared_ptr<atomic_flag>>(State::IsKeyTrigger).get()->test();
        }

        inline bool check_mouse_trigger() const
        {
            return state.get<shared_ptr<atomic_flag>>(State::IsMouseTrigger).get()->test();
        }

        static inline string str_time_now_bmp()
        {
            std::ostringstream oss;
            oss << system_clock::to_time_t(system_clock::now());
            oss << ".bmp";

            return oss.str();
        }
    };
}
