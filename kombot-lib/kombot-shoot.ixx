export module kombot:shoot;

import winapi;
using Winapi::Minwindef::Dword;
using Winapi::WinUser::Input::Input;
using Winapi::WinUser::Input::InputType;
using Winapi::WinUser::Input::MouseEventFlag;
using Winapi::WinUser::Input::send_input;

import :loop_on_thread;
using Kombot::LoopOnThread::LoopOnThread;

import :common;
using Kombot::Common::State;
using Kombot::Common::StateUser;

import std;
using std::atomic_flag;
using std::atomic;
using std::unreachable;

export namespace Kombot::Shoot
{
    enum class ShootMode
    {
        No, Always, OnTarget
    };

    class Shooter : public LoopOnThread, public StateUser
    {
    private:

        static const Dword start_shoot_flag = MouseEventFlag::MiddleDown;
        static const Dword stop_shoot_flag = MouseEventFlag::MiddleUp;

        atomic<ShootMode> mode;
        bool is_active;
        atomic_flag is_on_target;

    public:

        Shooter(State& state):
            StateUser(state),
            mode(ShootMode::No),
            is_active(false),
            is_on_target()
        { }

        Shooter(const Shooter& other) = delete;
        Shooter& operator=(const Shooter& other) = delete;

        Shooter(Shooter&& other) = default;
        Shooter& operator=(Shooter&& other) = default;

        inline void set_mode(ShootMode mode)
        {
            this->mode.store(mode);
        }

        inline void notify_on_target()
        {
            is_on_target.test_and_set();
        }

        inline void notify_off_target()
        {
            is_on_target.clear();
        }

    protected:

        bool iteration_condition() override
        {
            bool should_be_active = check_key_trigger() || check_mouse_trigger();
            bool is_changed = is_active != should_be_active;
            is_active = should_be_active;
            return is_changed;
        }

        void execute_iteration() override
        {
            if (is_active)
                activate();
            else
                deactivate();
        }

    private:

        inline void activate() const
        {
            switch (mode.load())
            {
            case ShootMode::No:
                break;
            case ShootMode::Always:
                send_mouse_input_with_flags(start_shoot_flag);
                break;
            case ShootMode::OnTarget:
                if (is_on_target.test())
                    send_mouse_input_with_flags(start_shoot_flag);
                break;
            default:
                unreachable();
            }
        }

        inline void deactivate() const
        {
            switch (mode.load())
            {
            case ShootMode::No:
                send_mouse_input_with_flags(stop_shoot_flag);
                break;
            case ShootMode::Always:
                send_mouse_input_with_flags(stop_shoot_flag);
                break;
            case ShootMode::OnTarget:
                send_mouse_input_with_flags(stop_shoot_flag);
                break;
            default:
                unreachable();
            }
        }

        static inline void send_mouse_input_with_flags(Dword flags)
        {
            Input input { };
            input.type = static_cast<Dword>(InputType::Mouse);
            input.mi.dwFlags = flags;
            send_input(1, &input);
        }
    };
}