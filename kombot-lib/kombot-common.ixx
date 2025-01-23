export module kombot:common;

import winapi;
using Winapi::WinUser::Key::KeyEvent;
using Winapi::WinUser::Mouse::MouseEvent;

import std;
using std::unordered_map;
using std::any;
using std::any_cast;
using std::string;
using std::function;
using std::string;
using std::shared_ptr;
using std::atomic_flag;

export namespace Kombot::Common
{
    using KeyEventHandler = function<void(const KeyEvent&)>;
    using MouseEventHandler = function<void(const MouseEvent&)>;

    struct State
    {
    private:

        unordered_map<string, any> storage;

    public:

        static string IsKeyTrigger;
        static string IsMouseTrigger;

        State():
            storage()
        { }

        State(unordered_map<string, any>&& storage):
            storage(storage)
        { }

        State(const State& other) = default;
        State& operator=(const State& other) = default;

        State(State&& other) = default;
        State& operator=(State&& other) = default;

        template<typename T>
        inline T& get(const string& name)
        {
            return any_cast<T&>(storage.at(name));
        }
    };
    string State::IsKeyTrigger = "is_key_trigger";
    string State::IsMouseTrigger = "is_mouse_trigger";

    class StateUser
    {
    protected:

        State& state;

    public:

        StateUser(State& state):
            state(state)
        { }

        StateUser(const StateUser& other) = default;
        StateUser& operator=(const StateUser& other) = default;

        StateUser(StateUser&& other) = default;
        StateUser& operator=(StateUser&& other) = default;

    protected:

        inline bool check_key_trigger() const
        {
            return state.get<shared_ptr<atomic_flag>>(State::IsKeyTrigger).get()->test();
        }

        inline bool check_mouse_trigger() const
        {
            return state.get<shared_ptr<atomic_flag>>(State::IsMouseTrigger).get()->test();
        }
    };
}
