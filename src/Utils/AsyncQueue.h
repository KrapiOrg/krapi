#pragma once


#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>

namespace krapi {

    template<typename T>
    class [[nodiscard]] MultiFuture {
    public:

        MultiFuture() = default;


        [[nodiscard]] std::conditional_t<std::is_void_v<T>, void, std::vector<T>> get() {

            if constexpr (std::is_void_v<T>) {
                for (size_t i = 0; i < futures.size(); ++i)
                    futures[i].get();
                return;
            } else {
                std::vector<T> results(futures.size());
                for (size_t i = 0; i < futures.size(); ++i)
                    results[i] = futures[i].get();
                return results;
            }
        }

        [[nodiscard]] std::shared_future<T> &operator[](const size_t i) {

            return futures[i];
        }

        void push_back(std::shared_future<T> future) {

            futures.push_back(std::move(future));
        }

        [[nodiscard]] size_t size() const {

            return futures.size();
        }

        void wait() const {

            for (size_t i = 0; i < futures.size(); ++i)
                futures[i].wait();
        }

    private:

        std::vector<std::shared_future<T>> futures;
    };

    class [[nodiscard]] AsyncQueue {
    public:

        AsyncQueue() : running(true) {

            thread = std::make_unique<std::thread>(&AsyncQueue::worker, this);
        }

        ~AsyncQueue() {

            wait_for_tasks();
            destroy();
        }


        template<typename F, typename... A>
        void push_task(F &&task, A &&... args) {

            std::function<void()> task_function = std::bind(std::forward<F>(task), std::forward<A>(args)...);
            {
                const std::scoped_lock tasks_lock(tasks_mutex);
                tasks.push(task_function);
            }
            ++tasks_total;
            task_available_cv.notify_one();
        }

        template<typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
        [[nodiscard]] std::shared_future<R> submit(F &&task, A &&... args) {

            std::function<R()> task_function = std::bind(std::forward<F>(task), std::forward<A>(args)...);
            std::shared_ptr<std::promise<R>> task_promise = std::make_shared<std::promise<R>>();
            push_task(
                    [task_function, task_promise] {
                        try {
                            if constexpr (std::is_void_v<R>) {
                                std::invoke(task_function);
                                task_promise->set_value();
                            } else {
                                task_promise->set_value(std::invoke(task_function));
                            }
                        }
                        catch (...) {
                            try {
                                task_promise->set_exception(std::current_exception());
                            }
                            catch (...) {
                            }
                        }
                    });
            return task_promise->get_future();
        }

        void wait_for_tasks() {

            waiting = true;
            std::unique_lock<std::mutex> tasks_lock(tasks_mutex);
            task_done_cv.wait(tasks_lock, [this] { return (tasks_total == 0); });
            waiting = false;
        }

    private:

        void destroy() {

            running = false;
            task_available_cv.notify_one();
            thread->join();
        }

        void worker() {

            while (running) {

                std::function<void()> task;
                std::unique_lock<std::mutex> tasks_lock(tasks_mutex);
                task_available_cv.wait(tasks_lock, [this] { return !tasks.empty() || !running; });

                if (running) {
                    task = std::move(tasks.front());
                    tasks.pop();
                    tasks_lock.unlock();
                    task();
                    tasks_lock.lock();
                    --tasks_total;
                    if (waiting)
                        task_done_cv.notify_one();
                }
            }
        }

        std::atomic<bool> running;
        std::condition_variable task_available_cv = {};
        std::condition_variable task_done_cv = {};
        std::queue<std::function<void()>> tasks = {};
        std::atomic<size_t> tasks_total = 0;
        mutable std::mutex tasks_mutex = {};
        std::unique_ptr<std::thread> thread = nullptr;
        std::atomic<bool> waiting = false;
    };

}