import winapi;
using Winapi::Minwindef::Bool;
using Winapi::Minwindef::False;
using Winapi::Minwindef::True;
using Winapi::Minwindef::Dword;
using Winapi::Minwindef::Long;
using Winapi::Consoleapi::ControlEvent;
using Winapi::Consoleapi::set_console_control_handler;
using Winapi::WinUser::Key::KeyEvent;
using Winapi::WinUser::Key::KeyEventType;
using Winapi::WinUser::Input::Input;
using Winapi::WinUser::Input::InputType;
using Winapi::WinUser::Input::MouseEventFlag;
using Winapi::WinUser::Input::send_input;

import kombot;
using Kombot::Common::State;
using Kombot::KVMS::InputListener;
using Kombot::KVMS::KVMSHook;
using Kombot::Decide::Decider;
using Kombot::Decide::Decision;
using Kombot::LoopOnThread::LoopOnThread;

import std;
using std::atomic_flag;
using std::thread;
using std::println, std::cerr;
using std::unordered_map;
using std::string;
using std::any;
using std::optional, std::nullopt;
using std::strtol;
using std::format;

namespace PixelsPerDegreeCalculator
{
    static const double x_total_rotation_degree = 360;
    static const double x_test_rotation_degree = 180;
    static const double y_total_rotation_degree = 90;
    static const double y_test_rotation_degree = 45;

    static const Dword start_move_x_vkcode = 39; // right arrow
    static const Dword end_move_x_vkcode = 37;   // left arrow
    static const Dword start_move_y_vkcode = 40; // down arrow
    static const Dword end_move_y_vkcode = 38;   // up arrow
    static const Dword test_x_vkcode = 'J';       
    static const Dword test_y_vkcode = 'K';       
    static const Dword print_vkcode = 'P';       
    static const Dword zero_vkcode = '0';        
    static const Dword inverse_test_vkcode = 'I';

    static const string x_total_px_count = "x_total_px_count";
    static const string y_total_px_count = "y_total_px_count";
    static const string total_mouse_move_count = "total_mouse_move_count";
    static const string sign = "sign";

    static const string usage = R"(PixelsPerDegreeCalculator keymaps:
r_arrow : start horizontal; l_arrow : cancel horizontal;
d_arrow : start vertical; u_arrow : stop vertical
J : test horizontal movement
K : test vertical movement
P : print state
0 : zero state
I : inverse delta sign)";

    class Rotator : public LoopOnThread
    {
    private:

        State& state;
        Dword dx;
        Dword dy;
        atomic_flag is_moving;

    public:

        Rotator() = delete;

        Rotator(State& state):
            state(state)
        { }

        inline void start_with_d(Dword dx, Dword dy)
        {
            this->dx = dx;
            this->dy = dy;
            is_moving.test_and_set();
        }

        inline void stop()
        {
            is_moving.clear();
        }

    protected:

        bool iteration_condition() override
        {
            return is_moving.test();
        }

        void execute_iteration() override
        {
            Input input { };
            input.type = static_cast<Dword>(InputType::Mouse);
            input.mi.dwFlags = MouseEventFlag::Move;
            input.mi.dx = dx;
            input.mi.dy = dy;
            send_input(1, &input);

            state.get<Long>(x_total_px_count) += dx;
            state.get<Long>(y_total_px_count) += dy;
            state.get<size_t>(total_mouse_move_count)++;
        }
    };

    struct Resource
    {
        State* state;
        Decider* decider;
        InputListener* input_listener;
        Rotator* rotator;
    };

    static Resource resource;

    struct Args
    {
        Long x_px_delta;
        Long y_px_delta;
    };

    static inline void free_playground()
    {
        delete resource.input_listener;
        delete resource.rotator;
        delete resource.decider;
        delete resource.state;
    }

    static Bool exit_handler(Dword control_event)
    {
        if (
            control_event == static_cast<Dword>(ControlEvent::C) ||
            control_event == static_cast<Dword>(ControlEvent::Close) ||
            control_event == static_cast<Dword>(ControlEvent::Logoff) ||
            control_event == static_cast<Dword>(ControlEvent::Shutdown)
        ) {
            free_playground();
        }
        return False;
    }

    static inline double x_ppd(State& state)
    {
        return static_cast<double>(state.get<Long>(x_total_px_count)) / x_total_rotation_degree;
    }

    static inline double y_ppd(State& state)
    {
        return static_cast<double>(state.get<Long>(y_total_px_count)) / y_total_rotation_degree;
    }

    static inline void print_state(State& state)
    {
        println("{} = {}", x_total_px_count, state.get<Long>(x_total_px_count));
        println("{} = {}", y_total_px_count, state.get<Long>(y_total_px_count));
        println("{} = {}", total_mouse_move_count, state.get<size_t>(total_mouse_move_count));
        println("{} = {}", sign, state.get<Long>(sign));
        println("x_ppd = {}", x_ppd(state));
        println("y_ppd = {}", y_ppd(state));
    }

    static void run(const Args args)
    {
        println("{}", usage);

        if (set_console_control_handler(exit_handler, True) != 0)
        {
            println(cerr, "{}", "Failed to register playground destructor using atexit");
            return;
        }

        resource.state = new State
        {
            unordered_map<string, any>
            {
                { x_total_px_count, static_cast<Long>(0) },
                { y_total_px_count, static_cast<Long>(0) },
                { total_mouse_move_count, static_cast<size_t>(0) },
                { sign, static_cast<Long>(1) }
            }
        };

        resource.decider = new Decider
        {
            *(resource.state),
            {
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down && event.info->vkCode == start_move_x_vkcode;
                    },
                    [&args](State& state, const KeyEvent& event)
                    {
                        Long delta = state.get<Long>(sign) * args.x_px_delta;
                        resource.rotator->start_with_d(delta, 0);
                    }
                ),
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Up && event.info->vkCode == end_move_x_vkcode;
                    },
                    [&args](State& state, const KeyEvent& event)
                    {
                        resource.rotator->stop();
                    }
                ),
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down && event.info->vkCode == test_x_vkcode;
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        Input input { };
                        input.type = static_cast<Dword>(InputType::Mouse);
                        input.mi.dwFlags = MouseEventFlag::Move;
                        input.mi.dx = static_cast<Long>(state.get<Long>(sign) * x_ppd(state) * x_test_rotation_degree);
                        input.mi.dy = 0;

                        println("send input dx = {}, dy = {}", input.mi.dx, input.mi.dy);
                        send_input(1, &input);
                    }
                ),
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down && event.info->vkCode == start_move_y_vkcode;
                    },
                    [&args](State& state, const KeyEvent& event)
                    {
                        Long delta = state.get<Long>(sign) * args.y_px_delta;
                        resource.rotator->start_with_d(0, delta);
                    }
                ),
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Up && event.info->vkCode == end_move_y_vkcode;
                    },
                    [&args](State& state, const KeyEvent& event)
                    {
                        resource.rotator->stop();
                    }
                ),
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down && event.info->vkCode == test_y_vkcode;
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        Input input { };
                        input.type = static_cast<Dword>(InputType::Mouse);
                        input.mi.dwFlags = MouseEventFlag::Move;
                        input.mi.dx = 0;
                        input.mi.dy = static_cast<Long>(state.get<Long>(sign) * y_ppd(state) * y_test_rotation_degree);

                        println("send input dx = {}, dy = {}", input.mi.dx, input.mi.dy);
                        send_input(1, &input);
                    }
                ),
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Up && event.info->vkCode == print_vkcode;
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        print_state(state);
                    }
                ),
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down && event.info->vkCode == zero_vkcode;
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        state.get<Long>(x_total_px_count) = 0;
                        state.get<Long>(y_total_px_count) = 0;
                        state.get<size_t>(total_mouse_move_count) = 0;
                        println("Zeroed state");
                    }
                ),
                Decision<KeyEvent>(
                    [](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down && event.info->vkCode == inverse_test_vkcode;
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        state.get<Long>(sign) *= -1;
                        println("Inverted sign");
                    }
                ),
            },
            { }
        };

        KVMSHook::initialize(
            resource.decider->get_key_event_handler(),
            nullptr
        );

        resource.input_listener = new InputListener(KVMSHook::get_hook_proc());
        resource.rotator = new Rotator(*(resource.state));

        Winapi::WinUser::Msg::start_message_loop();
    }
}

using PixelsPerDegreeCalculator::Args;

static const Long min_px_delta = 1;
static const Long max_px_delta = 200;
static const string usage = format(
    "PixelsPerDegreeCalculator [{} <= long x_px_delta <= {}] [{} <= long y_px_delta <= {}]",
    min_px_delta, max_px_delta,
    min_px_delta, max_px_delta
);

optional<Args> parse_args(int argc, char** argv)
{
    if (argc != 3)
    {
        println(cerr, "{}", usage);
        return nullopt;
    }

    Args args;

    char* end_x; char* end_y;
    args.x_px_delta = strtol(argv[1], &end_x, 10);
    args.y_px_delta = strtol(argv[2], &end_y, 10);
    if (*end_x != '\0' || *end_y != '\0')
    {
        println(cerr, "{}", usage);
        return nullopt;
    }

    println(
        "parsed args: x_px_delta = {}, y_px_delta = {}",
        args.x_px_delta, args.y_px_delta
    );

    if (args.x_px_delta < min_px_delta || max_px_delta < args.x_px_delta ||
        args.y_px_delta < min_px_delta || max_px_delta < args.y_px_delta)
    {
        println(cerr, "{}", usage);
        return nullopt;
    }

    return optional { args };
}

int main(int argc, char** argv)
{
    return parse_args(argc, argv)
        .transform([](Args args) { PixelsPerDegreeCalculator::run(args); return 0; })
        .value_or(1);
}