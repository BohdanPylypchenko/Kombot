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

        bool is_active;

        ShootMode mode;
        atomic_flag is_mode_changed;

        atomic_flag is_on_target;
        atomic_flag is_on_target_changed;

    public:

        Shooter(State& state):
            StateUser(state),
            is_active(false),
            mode(ShootMode::No),
            is_mode_changed(),
            is_on_target(),
            is_on_target_changed()
        { }

        Shooter(const Shooter& other) = delete;
        Shooter& operator=(const Shooter& other) = delete;

        Shooter(Shooter&& other) = default;
        Shooter& operator=(Shooter&& other) = default;

        inline void set_mode(ShootMode mode)
        {
            this->mode = mode;
            is_mode_changed.test_and_set();
        }

        inline void notify_on_target()
        {
            is_on_target.test_and_set();
            is_on_target_changed.test_and_set();
        }

        inline void notify_off_target()
        {
            is_on_target.clear();
            is_on_target_changed.test_and_set();
        }

    protected:

        void refresh_internal_state() override
        {
            is_active = false;

            is_mode_changed.clear();

            is_on_target.clear();
            is_on_target_changed.clear();
        }

        bool iteration_condition() override
        {
            bool should_be_active = check_mouse_trigger();

            bool is_changed =
                is_active != should_be_active ||
                is_mode_changed.test() ||
                is_on_target_changed.test();

            is_active = should_be_active;

            is_mode_changed.clear();
            is_on_target_changed.clear();

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
            switch (mode)
            {
            case ShootMode::No:
                break;
            case ShootMode::Always:
                send_mouse_input_with_flags(start_shoot_flag);
                break;
            case ShootMode::OnTarget:
                if (is_on_target.test())
                    send_mouse_input_with_flags(start_shoot_flag);
                else
                    send_mouse_input_with_flags(stop_shoot_flag);
                break;
            default:
                unreachable();
            }
        }

        inline void deactivate() const
        {
            switch (mode)
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