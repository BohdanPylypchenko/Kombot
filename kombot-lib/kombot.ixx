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

export import :aim;
using Kombot::Aim::Aimer;
using Kombot::Aim::AimConfig;

import winapi;
using Winapi::Minwindef::Dword;
using Winapi::WinUser::Key::KeyEvent;
using Winapi::WinUser::Key::KeyEventType;
using Winapi::WinUser::Msg::start_message_loop;

import std;
using std::string;
using std::unordered_map;
using std::any;
using std::atomic_flag;
using std::shared_ptr, std::make_shared;

namespace Kombot
{
    struct Resource
    {
        State* state;
        Decider* decider;
        InputListener* input_listener;
        HdcScreen* screen;
        Aimer* aimer;
    };

    static Resource resource { };

    export struct Config
    {
        Dword aim_keycode;
        AimConfig aim_config;
    };

    export void initialize(const Config& config)
    {
        resource.state = new State
        {
            unordered_map<string, any>
            {
                { State::IsKeyTrigger, make_shared<atomic_flag>() },
                { State::IsMouseTrigger, make_shared<atomic_flag>() }
            }
        };

        resource.decider = new Decider
        {
            *(resource.state),
            {
                Decision<KeyEvent>(
                    [&config](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Down &&
                               event.info->vkCode == config.aim_keycode;
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        state.get<shared_ptr<atomic_flag>>(State::IsKeyTrigger).get()->test_and_set();
                    }
                ),
                Decision<KeyEvent>(
                    [&config](const State& state, const KeyEvent& event)
                    {
                        return event.type == KeyEventType::Up &&
                               event.info->vkCode == config.aim_keycode;
                    },
                    [](State& state, const KeyEvent& event)
                    {
                        state.get<shared_ptr<atomic_flag>>(State::IsKeyTrigger).get()->clear();
                    }
                )
            },
            { }
        };

        KVMSHook::initialize(
            resource.decider->get_key_event_handler(),
            nullptr
        );

        resource.input_listener = new InputListener(KVMSHook::get_hook_proc());

        resource.screen = new HdcScreen();

        resource.aimer = new Aimer(
            *resource.screen, *resource.state, config.aim_config
        );
    }

    export void start()
    {
        resource.aimer->run();
        start_message_loop();
    }

    export void cleanup()
    {
        resource.aimer->notify_on_kombot_end();
        delete resource.aimer;
        delete resource.screen;
        delete resource.decider;
        delete resource.state;
        delete resource.input_listener;
    }
}
