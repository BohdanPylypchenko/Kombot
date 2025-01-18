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
using Kombot::Frameshot::MdFrameView;
using Kombot::Frameshot::FrameShooter;

import std;
using std::atomic_flag;
using std::thread;
using std::fabs;
using std::abs;
using std::pow;
using std::sqrt;
using std::invalid_argument;
using std::vector;
using std::println;
using std::cerr;
using std::string;
using std::shared_ptr;
using std::span;
using std::min_element, std::max_element;

export namespace Kombot::Aim
{
    struct AimConfig
    {
        BgrPixel target_color;
        BgrPixel max_target_color_difference;
        int frame_half_wh_px;
        int screen_width_relation;
        int screen_height_relation;
        int mouse_x_ppd;
        int mouse_y_ppd;
        int horizontal_fov;
        double barrier_coefficient;
    };

    class Aimer
    {
    private:

        struct Point
        {
            double x;
            double y;

            Point() = default;

            Point(double x, double y):
                x(x), y(y)
            { }

            Point(const Point& other) = default;
            Point& operator=(const Point& other) = default;

            Point(Point&& other) = default;
            Point& operator=(Point&& other) = default;

            bool operator==(const Point& other) const
            {
                return x == other.x && y == other.y;
            }

            inline double m2() const
            {
                return sqrt(pow(x, 2) + pow(y, 2));
            }
        };

        struct Rectangle
        {
            Point left_upper;
            Point right_lower;

            Rectangle() = default;

            Rectangle(const Point& left_upper, const Point& right_lower):
                left_upper(left_upper),
                right_lower(right_lower)
            { }

            Rectangle(const Rectangle& other) = default;
            Rectangle& operator=(const Rectangle& other) = default;

            Rectangle(Rectangle&& other) = default;
            Rectangle& operator=(Rectangle&& other) = default;

            inline bool is_point_in_rectangle(const Point& point) const
            {
                return
                    left_upper.x < point.x && point.x < right_lower.x &&
                    left_upper.y < point.y && point.y < right_lower.y;
            }

            static inline Rectangle as_bound_of_points(span<const Point> points)
            {
                auto compare_by_m2 = [](const Point& a, const Point& b)
                {
                    return a.m2() < b.m2();
                };

                Point min = *min_element(points.begin(), points.end(), compare_by_m2);
                Point max = *max_element(points.begin(), points.end(), compare_by_m2);

                return Rectangle(min, max);
            }
        };

        class AimVectorCalculator
        {
        private:

            int half_frame_wh;

            bool is_previous_move_big;
            int small_move_counter;
            double x_coefficient;
            double y_coefficient;
            double barrier2;

        public:

            AimVectorCalculator() = delete;

            AimVectorCalculator(
                int half_frame_wh,
                int screen_resolution_w,
                int screen_resolution_h,
                int horizontal_fov,
                int screen_width_relation,
                int screen_height_relation,
                int mouse_x_ppd,
                int mouse_y_ppd,
                double barrier_coefficient
            ):
                half_frame_wh(half_frame_wh),
                is_previous_move_big(false),
                small_move_counter(0)
            {
                double screen_pixels_per_degree =
                    (double)(screen_resolution_w) /
                    (double)(horizontal_fov);
                double aim_vertical_fov =
                    (double)(horizontal_fov) /
                    (double)(screen_width_relation) *
                    (double)(screen_height_relation);
                double t2 = (double)screen_resolution_h /
                                    aim_vertical_fov;
                if (fabs(screen_pixels_per_degree - t2) > 0.001)
                {
                    throw invalid_argument
                    {
                        "Error: failed screen pixels per degree relation check."
                    };
                }

                x_coefficient = (double)mouse_x_ppd / screen_pixels_per_degree;
                y_coefficient = (double)mouse_y_ppd / screen_pixels_per_degree;

                barrier2 = sqrt(barrier_coefficient * (pow(x_coefficient, 2) + pow(y_coefficient, 2)));
            }

            AimVectorCalculator(const AimVectorCalculator& other) = delete;
            AimVectorCalculator& operator=(const AimVectorCalculator& other) = delete;

            AimVectorCalculator(AimVectorCalculator&& other) = delete;
            AimVectorCalculator& operator=(AimVectorCalculator&& other) = delete;

            inline Point convert_to_aim_vector_simple(const Point& target_point)
            {
                return Point
                {
                    target_point.x - half_frame_wh,
                    target_point.y - half_frame_wh
                };
            }

            inline Point convert_to_aim_vector_scale(const Point& target_point)
            {
                small_move_counter++;

                Point result = convert_to_aim_vector_simple(target_point);

                if (result.m2() > barrier2 &&
                    !is_previous_move_big)
                {
                    result.x *= x_coefficient;
                    result.y *= y_coefficient;

                    is_previous_move_big = true;
                    zero_small_move_counter();
                }
                else
                {
                    result.x *= small_coefficient();
                    result.y *= small_coefficient();
                    is_previous_move_big = false;
                }

                return result;
            }

            inline void zero_small_move_counter() noexcept
            {
                small_move_counter = 0;
            }

            inline double small_coefficient() noexcept
            {
                //double result = small_move_counter * small_move_counter;
                //return result > 25 ? 25 : result;
                return 25;
            }
        };

        atomic_flag is_kombot_running;
        atomic_flag is_aiming;
        State& state;

        int screen_resolution_w;
        int screen_resolution_h;
        int half_frame_wh;
        int frame_wh;
        AimVectorCalculator avc;

        BgrPixel target_color;
        BgrPixel max_target_color_difference;
        vector<Point> target;

        Point previous_target_average;

        FrameShooter frame_shooter;

#ifdef KOMBOT_SAVE_FRAME
        size_t frame_count;
#endif

    public:

        Aimer(HdcScreen& screen, State& state, AimConfig config):
            is_kombot_running(),
            is_aiming(),
            state(state),
            screen_resolution_w(get_device_caps(screen.get(), DeviceCap::HorizontalResolution)),
            screen_resolution_h(get_device_caps(screen.get(), DeviceCap::VecticalResolution)),
            half_frame_wh(config.frame_half_wh_px),
            frame_wh(half_frame_wh * 2),
            avc
            {
                half_frame_wh,
                screen_resolution_w,
                screen_resolution_h,
                config.horizontal_fov,
                config.screen_width_relation,
                config.screen_height_relation,
                config.mouse_x_ppd,
                config.mouse_y_ppd,
                config.barrier_coefficient
            },
            target_color(config.target_color),
            max_target_color_difference(config.max_target_color_difference),
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

            target.reserve(frame_wh * frame_wh);

#ifdef KOMBOT_SAVE_FRAME
            frame_count = 0;
#endif
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
#ifdef KOMBOT_PERFORMANCE
                            auto start = std::chrono::high_resolution_clock::now();
#endif
                            target.clear();
                            aim_iteration();
#ifdef KOMBOT_PERFORMANCE
                            auto end = std::chrono::high_resolution_clock::now();
                            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                            auto count_per_second = 1000.0 / static_cast<double>(duration);
                            println("frame per second = {}", count_per_second);
#endif
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

        inline void aim_iteration()
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

#ifdef KOMBOT_LOG
            println("target size = {}", target.size());
#endif

            if (target.empty())
                return;

            Point current_target_average = target_average();
            if (previous_target_average == current_target_average)
                return;

            Rectangle bound = Rectangle::as_bound_of_points(target);
            if (bound.is_point_in_rectangle(Point(half_frame_wh, half_frame_wh)))
            {
                avc.zero_small_move_counter();
                //aim_with_vector(avc.convert_to_aim_vector_simple(current_target_average));
            }
            else
            {
                aim_with_vector(avc.convert_to_aim_vector_scale(current_target_average));
            }

#ifdef KOMBOT_SAVE_FRAME
            frame_shooter.dump_to_file(std::format("frame-original-{}-{}.bmp", frame_count, time()));

            MdFrameView md_frame_view = frame_shooter.get_md_frame_view();

            for (BgrPixel& current : frame_shooter.get_sd_frame_view())
            {
                if (!is_pixel_same_as_target(current))
                {
                    current = BgrPixel
                    { .blue = 0, .green = 0, .red = 0 };
                }
            }
            md_frame_view[current_target_average.y, current_target_average.x] = BgrPixel
            { .blue = 255, .green = 0, .red = 0 };
            md_frame_view[bound.left_upper.y, bound.left_upper.x] = BgrPixel
            { .blue = 0, .green = 255, .red = 0 };
            md_frame_view[bound.right_lower.y, bound.right_lower.x] = BgrPixel
            { .blue = 0, .green = 255, .red = 0 };

            frame_shooter.dump_to_file(std::format("frame-altered-{}-{}.bmp", frame_count, time()));
            frame_count++;
#endif

            previous_target_average = current_target_average;
        }

        inline void detect_target()
        {
            MdFrameView md_frame_view = frame_shooter.get_md_frame_view();
            for (auto y = 0; y != md_frame_view.extent(0); y++)
            {
                for (auto x = 0; x != md_frame_view.extent(1); x++)
                {
                    const BgrPixel& current = md_frame_view[y, x];
                    if (is_pixel_same_as_target(current))
                        target.push_back(Point(x, y));
                }
            }
        }

        inline bool is_pixel_same_as_target(const BgrPixel& pixel) const
        {
            return
                abs(pixel.blue - target_color.blue) <= max_target_color_difference.blue &&
                abs(pixel.green - target_color.green) <= max_target_color_difference.green &&
                abs(pixel.red - target_color.red) <= max_target_color_difference.red;
        }

        inline Point target_average() const
        {
            double sum_x = 0;
            double sum_y = 0;

            for (const auto& p : target)
            {
                sum_x += p.x;
                sum_y += p.y;
            }

            return Point(
                sum_x / (double)target.size(),
                sum_y / (double)target.size()
            );
        }

        static inline void aim_with_vector(const Point& point)
        {
            Input input { };
            input.type = InputMouseType;
            input.mi.dwFlags = MouseEventMove;
            input.mi.dx = static_cast<int>(point.x);
            input.mi.dy = static_cast<int>(point.y);
            send_input(1, &input);
#ifdef KOMBOT_LOG
            println("send input: x = {}, y = {}", input.mi.dx, input.mi.dy);
#endif
        }

        inline bool check_key_trigger() const
        {
            return state.get<shared_ptr<atomic_flag>>(State::IsKeyTrigger).get()->test();
        }

        inline bool check_mouse_trigger() const
        {
            return state.get<shared_ptr<atomic_flag>>(State::IsMouseTrigger).get()->test();
        }

#ifdef KOMBOT_SAVE_FRAME
        static inline std::time_t time()
        {
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
#endif
    };
}
