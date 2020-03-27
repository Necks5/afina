#ifndef AFINA_CONCURRENCY_EXECUTOR_H
#define AFINA_CONCURRENCY_EXECUTOR_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace Afina {
namespace Concurrency {

class Executor;

void perform(Executor* executor);
/**
 * # Thread pool
 */
class Executor {
public:
    enum class State {
        // Threadpool is fully operational, tasks could be added and get executed
        kRun,

        // Threadpool is on the way to be shutdown, no ned task could be added, but existing will be
        // completed as requested
        kStopping,

        // Threadppol is stopped
        kStopped
    };

    Executor(int low_watermark, int hight_watermark, int max_queue_size);
    ~Executor();


    /**
     * Signal thread pool to stop, it will stop accepting new jobs and close threads just after each become
     * free. All enqueued jobs will be complete.
     *
     * In case if await flag is true, call won't return until all background jobs are done and all threads are stopped
     */
    void Stop(bool await = false);

    template <typename F, typename... Types> bool Execute(F &&func, Types... args) {
        // Prepare "task"
        auto exec = std::bind(std::forward<F>(func), std::forward<Types>(args)...);

        std::unique_lock<std::mutex> lock(this->_mutex);
        if (_state != State::kRun || _tasks.size() == _max_queue_size) {
            return false;
        }

        if(_now_working == _working_threads && _working_threads < _hight_watermark) {
            std::thread(perform, this).detach();
            _working_threads++;
        }
        _tasks.push_back(exec);
        _empty_condition.notify_one();
        return true;
    }

private:
    // No copy/move/assign allowed
    Executor(const Executor &);            // = delete;
    Executor(Executor &&);                 // = delete;
    Executor &operator=(const Executor &); // = delete;
    Executor &operator=(Executor &&);      // = delete;

    /**
     * Main function that all pool threads are running. It polls internal task queue and execute tasks
     */
    friend void perform(Executor *executor);

    /**
     * Mutex to protect state below from concurrent modification
     */
    std::mutex _mutex;

    /**
     * Conditional variable to await new data in case of empty queue
     */
    std::condition_variable _empty_condition;

    /**
     * Vector of actual threads that perorm execution
     */
//    std::vector<std::thread> _threads;
//    problems with deleting thread

    /**
     * Task queue
     */
    std::deque<std::function<void()>> _tasks;

    /**
     * Flag to stop bg threads
     */
    State _state;

    int _now_working;
    int _working_threads;
    int _low_watermark;
    int _hight_watermark;
    int _max_queue_size;

    std::chrono::milliseconds _idle_time;


};

} // namespace Concurrency
} // namespace Afina

#endif // AFINA_CONCURRENCY_EXECUTOR_H
