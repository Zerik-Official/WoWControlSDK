#pragma once

#include "utils/json/Json.h"
#include <queue>
#include <mutex>
#include <future>
#include <chrono>
#include <functional>
#include <string>

namespace Utils
{

class TaskQueue
{
public:
    using Task = std::function<std::string()>;
    using VoidTask = std::function<void()>;

    void post(VoidTask task)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_voidQueue.push(std::move(task));
    }

    std::string execute(Task task, int timeoutMs)
    {
        std::packaged_task<std::string()> wrapped(std::move(task));
        auto future = wrapped.get_future();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_taskQueue.push(std::move(wrapped));
        }

        if (future.wait_for(std::chrono::milliseconds(timeoutMs)) == std::future_status::timeout)
            return SDK::makeErrorJson("timeout").dump();

        return future.get();
    }

    void drainAll()
    {
        std::queue<VoidTask> voidLocal;
        std::queue<std::packaged_task<std::string()>> taskLocal;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::swap(voidLocal, m_voidQueue);
            std::swap(taskLocal, m_taskQueue);
        }

        while (!voidLocal.empty())
        {
            auto task = std::move(voidLocal.front());
            voidLocal.pop();
            task();
        }
        while (!taskLocal.empty())
        {
            auto task = std::move(taskLocal.front());
            taskLocal.pop();
            task();
        }
    }

private:
    std::queue<VoidTask> m_voidQueue;
    std::queue<std::packaged_task<std::string()>> m_taskQueue;
    std::mutex m_mutex;
};

}
