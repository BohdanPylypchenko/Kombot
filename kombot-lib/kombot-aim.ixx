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
        double small_x_coefficient;
        double small_y_coefficient;
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
        };

        Shooter& shooter;

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

#ifdef KOMBOT_PERFORMANCE
        size_t total_frame_count;
#endif

#ifdef KOMBOT_SAVE_FRAME
        size_t frame_count;
#endif

    public:

        Aimer(HdcScreen& screen, State& state, Shooter& shooter, AimConfig config):
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
            target.reserve(frame_wh * frame_wh);

#ifdef KOMBOT_SAVE_FRAME
            frame_count = 0;
#endif
        }

        Aimer(const Aimer& other) = delete;
        Aimer& operator=(const Aimer& other) = delete;

        Aimer(Aimer&& other) = default;
        Aimer& operator=(Aimer&& other) = default;

    protected:

        bool iteration_condition() override
        {
            return check_mouse_trigger() && check_on_off_trigger();
        }

        void execute_iteration() override
        {
#ifdef KOMBOT_PERFORMANCE
            auto start = std::chrono::high_resolution_clock::now();
#endif
            target.clear();
            aim_iteration();
#ifdef KOMBOT_PERFORMANCE
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if (total_frame_count % 100 == 0)
            {
                auto count_per_second = 1000.0 / static_cast<double>(duration);
                println("frame per second = {}", count_per_second);
            }
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

            detect_target();

#ifdef KOMBOT_LOG
            println("target size = {}", target.size());
#endif

            if (target.empty())
            {
                shooter.notify_off_target();
                return;
            }

            Point current_target_average = target_average();
            if (previous_target_average == current_target_average)
                return;

            Rectangle bound = Rectangle::as_bound_of_points(target);
            if (bound.is_point_in_rectangle(Point(half_frame_wh, half_frame_wh)))
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
