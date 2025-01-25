export module kombot:aim;

import winapi;
using Winapi::Minwindef::Byte;
using Winapi::Minwindef::Dword;
using Winapi::WinUser::Input::Input;
using Winapi::WinUser::Input::InputType;
using Winapi::WinUser::Input::MouseEventFlag;
using Winapi::WinUser::Input::send_input;
using Winapi::Display::BgrPixel;
using Winapi::Display::get_device_caps;
using Winapi::Display::DeviceCap;

import :loop_on_thread;
using Kombot::LoopOnThread::LoopOnThread;

import :common;
using Kombot::Common::State;
using Kombot::Common::StateUser;

import :frameshot;
using Kombot::Frameshot::HdcScreen;
using Kombot::Frameshot::MdFrameView;
using Kombot::Frameshot::FrameShooter;

import :shoot;
using Kombot::Shoot::Shooter;

import std;
using std::atomic_flag;
using std::thread;
using std::fabs;
using std::abs;
using std::pow;
using std::sqrt;
using std::invalid_argument;
using std::println;
using std::cerr;
using std::string;
using std::format;
using std::shared_ptr;
using std::span;
using std::optional, std::nullopt, std::make_optional;
using std::string, std::ostringstream;

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
        double small_x_coefficient;
        double small_y_coefficient;

        inline string to_string() const
        {
            ostringstream oss { };

            oss <<
                "AimConfig: " << '\n' <<
                "blue: " << static_cast<int>(target_color.blue) << " +- " << static_cast<int>(max_target_color_difference.blue) << '\n' <<
                "green: " << static_cast<int>(target_color.green) << " +- " << static_cast<int>(max_target_color_difference.green) << '\n' <<
                "red: " << static_cast<int>(target_color.red) << " +- " << static_cast<int>(max_target_color_difference.red) << '\n' <<
                "frame_half_wh (in px): " << frame_half_wh_px << '\n' <<
                "screen_width_relation: " << screen_width_relation << '\n' <<
                "screen_height_relation: " << screen_height_relation << '\n' <<
                "mouse_x_ppd: " << mouse_x_ppd << '\n' <<
                "mouse_y_ppd: " << mouse_y_ppd << '\n' <<
                "horizontal_fov: " << horizontal_fov << '\n' <<
                "barrier_coefficient: " << barrier_coefficient << '\n' <<
                "small_x_coefficient: " << small_x_coefficient << '\n' <<
                "small_y_coefficient: " << small_y_coefficient;

            return oss.str();
        }
    };

    class Aimer : public LoopOnThread, public StateUser
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

            inline string to_string() const
            {
                return format("point x = {}, y = {}", x, y);
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
        };

        class AimVectorCalculator
        {
        private:

            int half_frame_wh;

            bool is_previous_move_big;
            double big_x_coefficient;
            double big_y_coefficient;
            double small_x_coefficient;
            double small_y_coefficient;
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
                double barrier_coefficient,
                double small_x_coefficient,
                double small_y_coefficient
            ):
                half_frame_wh(half_frame_wh),
                is_previous_move_big(false),
                small_x_coefficient(small_x_coefficient),
                small_y_coefficient(small_y_coefficient)
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

                big_x_coefficient = (double)mouse_x_ppd / screen_pixels_per_degree;
                big_y_coefficient = (double)mouse_y_ppd / screen_pixels_per_degree;

                barrier2 = barrier_coefficient * sqrt(pow(big_x_coefficient, 2) + pow(big_y_coefficient, 2));
            }

            AimVectorCalculator(const AimVectorCalculator& other) = delete;
            AimVectorCalculator& operator=(const AimVectorCalculator& other) = delete;

            AimVectorCalculator(AimVectorCalculator&& other) = default;
            AimVectorCalculator& operator=(AimVectorCalculator&& other) = default;

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
                Point result = convert_to_aim_vector_simple(target_point);

                if (result.m2() > barrier2 &&
                    !is_previous_move_big)
                {
                    result.x *= big_x_coefficient;
                    result.y *= big_y_coefficient;
                    is_previous_move_big = true;
                }
                else
                {
                    result.x *= small_x_coefficient;
                    result.y *= small_y_coefficient;
                    is_previous_move_big = false;
                }

                return result;
            }

            inline string to_string() const
            {
                ostringstream oss { };

                oss <<
                    "AimVectorCalculator:" << '\n' <<
                    "big_x_coefficient: " << big_x_coefficient << '\n' <<
                    "big_y_coefficient: " << big_y_coefficient << '\n' <<
                    "small_x_coefficient: " << small_x_coefficient << '\n' <<
                    "small_y_coefficient: " << small_y_coefficient << '\n' <<
                    "barrier2: " << barrier2;

                return oss.str();
            }
        };

        Shooter& shooter;

        int screen_resolution_w;
        int screen_resolution_h;
        int half_frame_wh;
        int frame_wh;
        AimVectorCalculator avc;

        BgrPixel target_color;
        BgrPixel max_target_color_difference;

        Point previous_target_average;

        FrameShooter frame_shooter;

#if defined(KOMBOT_PERFORMANCE) || defined(KOMBOT_SAVE_FRAME)
        size_t total_frame_count;
#endif

    public:

        Aimer(
            HdcScreen& screen, State& state, Shooter& shooter,
            AimConfig config
        ):
            StateUser(state),
            shooter(shooter),
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
                config.barrier_coefficient,
                config.small_x_coefficient,
                config.small_y_coefficient
            },
            target_color(config.target_color),
            max_target_color_difference(config.max_target_color_difference),
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
#if defined(KOMBOT_PERFORMANCE) || defined(KOMBOT_SAVE_FRAME)
            total_frame_count = 0;
#endif
        }

        Aimer(const Aimer& other) = delete;
        Aimer& operator=(const Aimer& other) = delete;

        Aimer(Aimer&& other) = default;
        Aimer& operator=(Aimer&& other) = default;

        inline string to_string() const
        {
            ostringstream oss { };

            oss <<
                "Aimer: " << '\n' <<
                "blue: " << static_cast<int>(target_color.blue) << " +- " << static_cast<int>(max_target_color_difference.blue) << '\n' <<
                "green: " << static_cast<int>(target_color.green) << " +- " << static_cast<int>(max_target_color_difference.green) << '\n' <<
                "red: " << static_cast<int>(target_color.red) << " +- " << static_cast<int>(max_target_color_difference.red) << '\n' <<
                "screen_resolution_w: " << screen_resolution_w << '\n' <<
                "screen_resolution_h: " << screen_resolution_h << '\n' <<
                "half_frame_wh: " << half_frame_wh << '\n' <<
                "frame_wh: " << frame_wh << '\n' <<
                avc.to_string();

            return oss.str();
        }

    protected:

        void refresh_internal_state() override
        {
            previous_target_average = Point(half_frame_wh, half_frame_wh);
        }

        bool iteration_condition() override
        {
            return check_mouse_trigger();
        }

        void execute_iteration() override
        {
#ifdef KOMBOT_PERFORMANCE
            auto start = std::chrono::high_resolution_clock::now();
#endif
            aim_iteration();
#ifdef KOMBOT_PERFORMANCE
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if (total_frame_count % 100 == 0)
            {
                auto count_per_second = 1000.0 / static_cast<double>(duration);
                println("frame per second = {}", count_per_second);
            }
#endif
#if defined(KOMBOT_PERFORMANCE) || defined(KOMBOT_SAVE_FRAME)
            total_frame_count++;
#endif
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

            optional<Rectangle> target = detect_target();
            if (!target.has_value())
            {
                shooter.notify_off_target();
#ifdef KOMBOT_LOG
                println("Target empty");
#endif
                return;
            }

            Point current_target_average = target_average(*target);
            if (previous_target_average == current_target_average)
                return;

#ifdef KOMBOT_LOG
            println(
                "target: left upper = {}; right lower = {};",
                target->left_upper.to_string(),
                target->right_lower.to_string()
            );
            println("target_average = {}", current_target_average.to_string());
#endif

            if (target->is_point_in_rectangle(Point(half_frame_wh, half_frame_wh)))
            {
                shooter.notify_on_target();
            }
            else
            {
                shooter.notify_off_target();
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
#endif

            previous_target_average = current_target_average;
        }

        inline optional<Rectangle> detect_target()
        {
            MdFrameView md_frame_view = frame_shooter.get_md_frame_view();

            optional<Point> left_upper = find_left_upper_point(md_frame_view);
            if (!left_upper.has_value())
                return nullopt;

            optional<Point> right_lower = find_right_lower_point(md_frame_view);
            if (!left_upper.has_value())
                return nullopt;

            return make_optional<Rectangle>(*left_upper, *right_lower);
        }

        inline optional<Point> find_left_upper_point(const MdFrameView& md_frame_view) const
        {
            for (int y = 0; y != md_frame_view.extent(0); y++)
            {
                for (int x = 0; x != md_frame_view.extent(1); x++)
                {
                    const BgrPixel& current = md_frame_view[y, x];
                    if (is_pixel_same_as_target(current))
                        return make_optional<Point>(
                            static_cast<double>(x),
                            static_cast<double>(y)
                        );
                }
            }
            return nullopt;
        }

        inline optional<Point> find_right_lower_point(const MdFrameView& md_frame_view) const
        {
            for (int y = static_cast<int>(md_frame_view.extent(0)) - 1; y >= 0; y--)
            {
                for (int x = static_cast<int>(md_frame_view.extent(1)) - 1; x >= 0; x--)
                {
                    const BgrPixel& current = md_frame_view[y, x];
                    if (is_pixel_same_as_target(current))
                        return make_optional<Point>(
                            static_cast<double>(x),
                            static_cast<double>(y)
                        );
                }
            }
            return nullopt;
        }

        inline bool is_pixel_same_as_target(const BgrPixel& pixel) const
        {
            return
                abs(pixel.blue - target_color.blue) <= max_target_color_difference.blue &&
                abs(pixel.green - target_color.green) <= max_target_color_difference.green &&
                abs(pixel.red - target_color.red) <= max_target_color_difference.red;
        }

        static inline Point target_average(const Rectangle& target)
        {
            return Point(
                (double)(target.left_upper.x + target.right_lower.x) / 2.0,
                (double)(target.left_upper.y + target.right_lower.y) / 2.0
            );
        }

        static inline void aim_with_vector(const Point& point)
        {
            Input input { };
            input.type = static_cast<Dword>(InputType::Mouse);
            input.mi.dwFlags = MouseEventFlag::Move;
            input.mi.dx = static_cast<int>(point.x);
            input.mi.dy = static_cast<int>(point.y);
            send_input(1, &input);
#ifdef KOMBOT_LOG
            println("send input: x = {}, y = {}", input.mi.dx, input.mi.dy);
#endif
        }

#ifdef KOMBOT_SAVE_FRAME
        static inline std::time_t time()
        {
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
#endif
    };
}
