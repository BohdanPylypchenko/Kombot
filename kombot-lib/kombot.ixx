export module kombot;

export import :common;
using Kombot::Common::State;

export import :kvms;
using Kombot::KVMS::KVMSHook;
using Kombot::KVMS::InputListener;

export import :decide;
using Kombot::Decide::Decider;
using Kombot::Decide::Decision;

export import :frameshot;
using Kombot::Frameshot::HdcScreen;

export import :loop_on_thread;

export import :shoot;
using Kombot::Shoot::Shooter;
using Kombot::Shoot::ShootMode;

export import :aim;
using Kombot::Aim::Aimer;
using Kombot::Aim::AimConfig;

import winapi;
using Winapi::Minwindef::Dword;
using Winapi::WinUser::Key::KeyEvent;
using Winapi::WinUser::Key::KeyEventType;
using Winapi::WinUser::Mouse::MouseEvent;
using Winapi::WinUser::Mouse::MouseEventType;
using Winapi::WinUser::Msg::start_message_loop;

import std;
using std::string;
using std::unordered_map;
using std::any;
using std::atomic_flag;
using std::shared_ptr, std::make_shared;
using std::unordered_set;
using std::vector;
using std::println;
using std::unreachable;

namespace Kombot
{
    export namespace Konfig
    {
        using KeycodeSet = unordered_set<Dword>;

        enum class MouseTriggerType
        {
            Left, Right
        };

        struct Config
        {
            Dword on_off_keycode;
            AimConfig aim_config;
            KeycodeSet keycodes_no;
            KeycodeSet keycodes_always;
            KeycodeSet keycodes_on_target;
            MouseTriggerType mouse_trigger_type;
        };
    }

    using namespace Konfig;

    struct Resource
    {
        State* state;
        Decider* decider;
        InputListener* input_listener;
        HdcScreen* screen;
        Shooter* shooter;
        Aimer* aimer;
    };

    static Resource resource { };

    struct MouseEventTypePair
    {
        MouseEventType down;
        MouseEventType up;

        static MouseEventTypePair of_mouse_trigger_type(
            MouseTriggerType mouse_trigger_type
        ) {
            switch (mouse_trigger_type)
            {
            case MouseTriggerType::Left:
                return MouseEventTypePair
                {
                    .down = MouseEventType::LeftDown,
                    .up = MouseEventType::LeftUp
                };
            case MouseTriggerType::Right:
                return MouseEventTypePair
                {
                    .down = MouseEventType::RightDown,
                    .up = MouseEventType::RightUp
                };
            default:
                unreachable();
            }
        }
    };

    export void initialize(const Config& config)
    {
        resource.state = new State
        {
            unordered_map<string, any>
            {
                { State::OnOffTrigger, make_shared<atomic_flag>() },
                { State::IsMouseTrigger, make_shared<atomic_flag>() }
            }
        };

        MouseEventTypePair mouse_event_type_pair =
            MouseEventTypePair::of_mouse_trigger_type(config.mouse_trigger_type);

        resource.decider = new Decider
        {
            *resource.state,
            {
                Decision<KeyEvent>(
                    [&config](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Up &&
                               event.info->vkCode == config.on_off_keycode;
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        atomic_flag* on_off =
                            state.get<shared_ptr<atomic_flag>>(State::OnOffTrigger).get();
                        if (on_off->test())
                        {
                            on_off->clear();
                            println("kombot is off");
                        }
                        else
                        {
                            on_off->test_and_set();
                            println("kombot is on");
                        }
                    }
                ),
                Decision<KeyEvent>(
                    [&config](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down &&
                               config.keycodes_no.contains(event.info->vkCode);
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        resource.shooter->set_mode(ShootMode::No);
                        println("shoot mode = no");
                    }
                ),
                Decision<KeyEvent>(
                    [&config](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down &&
                               config.keycodes_always.contains(event.info->vkCode);
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        resource.shooter->set_mode(ShootMode::Always);
                        println("shoot mode = always");
                    }
                ),
                Decision<KeyEvent>(
                    [&config](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down &&
                               config.keycodes_on_target.contains(event.info->vkCode);
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        resource.shooter->set_mode(ShootMode::OnTarget);
                        println("shoot mode = on target");
                    }
                ),
            },
            {
                Decision<MouseEvent>(
                    [mouse_event_type_pair](const State& state, const MouseEvent& event)
                    {
                        return event.type == mouse_event_type_pair.down;
                    },
                    [](State& state, const MouseEvent& event)
                    {
                        state.get<shared_ptr<atomic_flag>>(State::IsMouseTrigger).get()->test_and_set();
                    }
                ),
                Decision<MouseEvent> (
                    [mouse_event_type_pair](const State& state, const MouseEvent& event)
                    {
                        return event.type == mouse_event_type_pair.up;
                    },
                    [](State& state, const MouseEvent& event)
                    {
                        state.get<shared_ptr<atomic_flag>>(State::IsMouseTrigger).get()->clear();
                    }
                )
            }
        };

        KVMSHook::initialize(
            resource.decider->get_key_event_handler(),
            resource.decider->get_mouse_event_handler()
        );

        resource.input_listener = new InputListener();

        resource.screen = new HdcScreen();

        resource.shooter = new Shooter(*resource.state);

        resource.aimer = new Aimer(
            *resource.screen, *resource.state, *resource.shooter, config.aim_config
        );

        println("Initialized kombot");
    }

    export void start()
    {
        println("Starting kombot");
        start_message_loop();
    }

    export void cleanup()
    {
        delete resource.aimer;
        delete resource.shooter;
        delete resource.screen;
        delete resource.decider;
        delete resource.state;
        delete resource.input_listener;
        println("Cleaned kombot");
    }
}
