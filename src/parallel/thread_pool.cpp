#include "parallel/thread_pool.hpp"

namespace cclint {
namespace parallel {

ThreadPool::ThreadPool(size_t num_threads) : stop_(false) {
    if (num_threads == 0) {
        num_threads = detect_cpu_cores();
    }

    // 最低1スレッド、最大でもCPUコア数x2まで
    num_threads = std::max<size_t>(1, num_threads);
    num_threads = std::min<size_t>(num_threads, detect_cpu_cores() * 2);

    workers_.reserve(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this] { this->worker_thread(); });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    condition_.notify_all();

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            condition_.wait(lock,
                            [this] { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty()) {
                return;
            }

            if (!tasks_.empty()) {
                task = std::move(tasks_.front());
                tasks_.pop();
            }
        }

        if (task) {
            task();
        }
    }
}

size_t ThreadPool::detect_cpu_cores() {
    unsigned int cores = std::thread::hardware_concurrency();
    return cores > 0 ? cores : 1;
}

} // namespace parallel
} // namespace cclint
