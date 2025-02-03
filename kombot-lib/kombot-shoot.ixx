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

        bool is_active;

        atomic<ShootMode> mode;
        atomic_flag is_mode_changed;

        atomic_flag is_on_target;
        atomic_flag is_on_target_changed;

#ifndef KOMBOT_MAYBE_SHOOT_ALWAYS
        atomic_flag is_shooting;
#endif

    public:

        Shooter(State& state):
            StateUser(state),
            is_active(false),
            mode(ShootMode::No),
            is_mode_changed(),
            is_on_target(),
            is_on_target_changed()
#ifndef KOMBOT_MAYBE_SHOOT_ALWAYS
            ,is_shooting()
#endif
        {}

        Shooter(const Shooter& other) = delete;
        Shooter& operator=(const Shooter& other) = delete;

        Shooter(Shooter&& other) = default;
        Shooter& operator=(Shooter&& other) = default;

        inline void set_mode(ShootMode mode)
        {
            this->mode.store(mode);
            is_mode_changed.test_and_set();
        }

        inline void notify_on_target()
        {
            if (mode.load() == ShootMode::OnTarget)
                maybe_start_shoot();
            is_on_target.test_and_set();
            is_on_target_changed.test_and_set();
        }

        inline void notify_off_target()
        {
            if (mode.load() == ShootMode::OnTarget)
                maybe_stop_shoot();
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

#ifndef KOMBOT_MAYBE_SHOOT_ALWAYS
            is_shooting.clear();
#endif
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

        inline void activate()
        {
            switch (mode.load())
            {
            case ShootMode::No:
                break;
            case ShootMode::Always:
                maybe_start_shoot();
                break;
            case ShootMode::OnTarget:
                if (is_on_target.test())
                    maybe_start_shoot();
                else
                    maybe_stop_shoot();
                break;
            default:
                unreachable();
            }
        }

        inline void deactivate()
        {
            switch (mode.load())
            {
            case ShootMode::No:
                maybe_stop_shoot();
                break;
            case ShootMode::Always:
                maybe_stop_shoot();
                break;
            case ShootMode::OnTarget:
                maybe_stop_shoot();
                break;
            default:
                unreachable();
            }
        }

        inline void maybe_start_shoot()
        {
#ifdef KOMBOT_MAYBE_SHOOT_ALWAYS
            send_mouse_input_with_flags(start_shoot_flag);
#else
            if (!is_shooting.test_and_set())
                send_mouse_input_with_flags(start_shoot_flag);
#endif
        }

        inline void maybe_stop_shoot()
        {
#ifdef KOMBOT_MAYBE_SHOOT_ALWAYS
            send_mouse_input_with_flags(stop_shoot_flag);
#else
            if (is_shooting.test())
            {
                send_mouse_input_with_flags(stop_shoot_flag);
                is_shooting.clear();
            }
#endif
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