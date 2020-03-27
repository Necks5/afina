#include <afina/concurrency/Executor.h>

namespace Afina {
namespace Concurrency {


    Executor::Executor(int low_watermark, int hight_watermark, int max_queue_size) : _state(State::kRun) {
        std::lock_guard<std::mutex> l(_mutex);
        _low_watermark = low_watermark;
        _hight_watermark = hight_watermark;
        _max_queue_size = max_queue_size;
        _idle_time = std::chrono::milliseconds(100);
        _working_threads = low_watermark;
        _now_working = low_watermark;
        for(int i = 0; i < _low_watermark; i++) {
            std::thread(perform, this).detach();
        }
    }

    Executor::~Executor() {
        Stop(true);
    }

    void Executor::Stop(bool await) {
        std::unique_lock<std::mutex> lock(_mutex);
        _state = State::kStopping;


        if (_now_working == 0) {
            _state = State::kStopped;
            return;
        }

        _empty_condition.notify_all();


    }


    void perform(Executor *executor) {
        std::function<void()> task;
        for(;;) {
            std::unique_lock<std::mutex> m(executor->_mutex);
            if(executor->_state == Executor::State::kStopping && executor->_tasks.size() == 0) {
                executor->_state = Executor::State::kStopped;
            }
            bool res = executor->_empty_condition.wait_for(m, executor->_idle_time,
                                                           [executor]{ return executor->_tasks.size() != 0 || executor->_state != Executor::State::kRun;});
            if(res) {
                task = executor->_tasks.front();
                executor->_tasks.pop_front();
                try {
                    executor->_now_working += 1;
                    task();
                    executor->_now_working -= 1;
                } catch (std::exception &ex) { }
            } else {
                if(executor->_working_threads > executor->_low_watermark || executor->_state == Executor::State::kStopped) {
                    break;
                }
            }

        }
        executor->_working_threads -= 1;


    }

}
} // namespace Afina
