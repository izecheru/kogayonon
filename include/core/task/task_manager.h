#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include "core/singleton/singleton.h"

namespace kogayonon
{

  class TaskManager :public Singleton<TaskManager>
  {
  public:
    explicit TaskManager(size_t threadCount = std::thread::hardware_concurrency());
    ~TaskManager();
    void enqueue(std::function<void()> task);
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
}
