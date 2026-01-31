#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace cclint {
namespace parallel {

/// シンプルなスレッドプール実装
class ThreadPool {
public:
    /// コンストラクタ
    /// @param num_threads スレッド数（0で自動検出）
    explicit ThreadPool(size_t num_threads = 0);

    /// デストラクタ
    ~ThreadPool();

    // コピー禁止
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /// タスクをキューに追加
    /// @param func 実行する関数
    /// @return future オブジェクト
    template <typename F, typename... Args>
    auto enqueue(F&& func,
                 Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

    /// スレッド数を取得
    size_t size() const { return workers_.size(); }

    /// CPU コア数を自動検出
    static size_t detect_cpu_cores();

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;

    /// ワーカースレッドの処理
    void worker_thread();
};

// テンプレート実装
template <typename F, typename... Args>
auto ThreadPool::enqueue(F&& func, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(func), std::forward<Args>(args)...));

    std::future<return_type> result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        if (stop_) {
            throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
        }

        tasks_.emplace([task]() { (*task)(); });
    }

    condition_.notify_one();
    return result;
}

}  // namespace parallel
}  // namespace cclint
