export module kombot:loop_on_thread;

import std;
using std::atomic_flag;
using std::thread;
using std::move;

export namespace Kombot::LoopOnThread
{
    class LoopOnThread
    {
    private:

        atomic_flag is_loop_running;
        thread loop_thread;

    public:

        LoopOnThread() = default;

        LoopOnThread(const LoopOnThread& other) = delete;
        LoopOnThread& operator=(const LoopOnThread& other) = delete;

        LoopOnThread(LoopOnThread&& other) noexcept:
            is_loop_running(),
            loop_thread(move(other.loop_thread))
        {
            if (other.is_loop_running.test())
                is_loop_running.test_and_set();
            other.is_loop_running.clear();
        }

        LoopOnThread& operator=(LoopOnThread&& other) noexcept
        {
            if (this != &other)
            {
                is_loop_running.clear();
                loop_thread.join();

                if (other.is_loop_running.test())
                    is_loop_running.test_and_set();
                else
                    is_loop_running.clear();

                loop_thread = move(other.loop_thread);

                other.is_loop_running.clear();
            }
            return *this;
        }

        virtual ~LoopOnThread()
        {
            stop();
        }

        inline void start()
        {
            if (is_loop_running.test())
                stop();

            refresh_internal_state();
            is_loop_running.test_and_set();

            loop_thread = thread
            {
                [this]()
                {
                    while (is_loop_running.test())
                    {
                        if (iteration_condition()) execute_iteration();
                    }
                }
            };
        }

        inline void stop()
        {
            is_loop_running.clear();
            if (loop_thread.joinable())
                loop_thread.join();
        }

    protected:

        virtual void refresh_internal_state() = 0;
        virtual bool iteration_condition() = 0;
        virtual void execute_iteration() = 0;

    };
}