#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

#include "core/singleton/singleton.h"

namespace kogayonon
{
  class TaskManager
  {
  public:
    explicit TaskManager(size_t thread_count = std::thread::hardware_concurrency());
    ~TaskManager();

    // void enqueue(std::function<void()> task);

    template <typename Func, typename... Args>
    auto enqueue(Func&& func, Args&&... args) -> std::future<std::invoke_result_t<Func, Args...>>
    {
      using return_type = std::invoke_result_t<Func, Args...>;
      auto p_task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

      {
        std::lock_guard lock(m_queue_mutex);
        if (m_stop)
        {
          throw std::runtime_error("Thread pool is stopped");
        }
        m_tasks.emplace([p_task]() { (*p_task)(); });
      }
      m_cvar.notify_one();
      return p_task->get_future();
    }
    void stop();

  private:
    void workerThread();

  private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queue_mutex;
    std::condition_variable m_cvar;
    std::atomic<bool> m_stop{false};
  };
} // namespace kogayonon
