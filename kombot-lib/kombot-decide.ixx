export module kombot:decide;

import winapi;
using Winapi::WinUser::Key::KeyEvent;
using Winapi::WinUser::Mouse::MouseEvent;

import :common;
using Kombot::Common::State;
using Kombot::Common::KeyEventHandler;
using Kombot::Common::MouseEventHandler;

import std;
using std::same_as;
using std::function;
using std::vector;
using std::initializer_list;
using std::span;

export namespace Kombot::Decide
{
    template<typename E>
    concept KeyOrMouseEvent = same_as<E, KeyEvent> || same_as<E, MouseEvent>;

    template<KeyOrMouseEvent E>
    using Condition = function<bool(const State&, const E&)>;
    
    template<KeyOrMouseEvent E>
    using Action = function<void(State&, const E&)>;

    template<KeyOrMouseEvent E>
    struct Decision
    {
    private:

        Condition<E> condition;
        Action<E> action;

    public:

        Decision(Condition<E> condition, Action<E> action):
            condition(condition),
            action(action)
        { }

        Decision(const Decision& other) = default;
        Decision& operator=(const Decision& other) = default;

        Decision(Decision&& other) = default;
        Decision& operator=(Decision&& other) = default;

        inline Action<E>& get_action()
        {
            return action;
        }

        inline Condition<E>& get_condition()
        {
            return condition;
        }
    };

    class Decider
    {
    private:

        State& state;
        vector<Decision<KeyEvent>> key_decisions;
        vector<Decision<MouseEvent>> mouse_decisions;

    public:

        Decider(
            State& state,
            initializer_list<Decision<KeyEvent>> key_decisions,
            initializer_list<Decision<MouseEvent>> mouse_decisions
        ):
            state(state),
            key_decisions(key_decisions),
            mouse_decisions(mouse_decisions)
        { }

        Decider(
            State& state,
            vector<Decision<KeyEvent>>& key_decisions,
            vector<Decision<MouseEvent>>& mouse_decisions
        ):
            state(state),
            key_decisions(key_decisions),
            mouse_decisions(mouse_decisions)
        { }

        Decider(const Decider& other) = default;
        Decider& operator=(const Decider& other) = default;

        Decider(Decider&& other) = default;
        Decider& operator=(Decider&& other) = default;

        inline KeyEventHandler get_key_event_handler()
        {
            return [this](const KeyEvent& event)
            {
                handle_key_event(event);
            };
        }

        inline MouseEventHandler get_mouse_event_handler()
        {
            return [this](const MouseEvent& event)
            {
                handle_mouse_event(event);
            };
        }

    private:

        void handle_key_event(const KeyEvent& event)
        {
            handle_event<KeyEvent>(
                state, event,
                span<Decision<KeyEvent>>(key_decisions.begin(), key_decisions.end())
            );
        }

        void handle_mouse_event(const MouseEvent& event)
        {
            handle_event<MouseEvent>(
                state, event,
                span<Decision<MouseEvent>>(mouse_decisions.begin(), mouse_decisions.end())
            );
        }

        template<KeyOrMouseEvent E>
        static void handle_event(State& state, const E& event, const span<Decision<E>> decisions)
        {
            for (Decision<E>& d : decisions)
            {
                if (d.get_condition()(state, event)) d.get_action()(state, event);
            }
        }
    };
}
