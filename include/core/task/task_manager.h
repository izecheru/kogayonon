#pragma once
#include "core/singleton/singleton.h"
#include "core/logger.h"
#include <queue>
#include <future>

namespace kogayonon
{
  class TaskManager : public Singleton<TaskManager>
  {
  public:
    bool completed();

    // we call the function callable in async mode and pass the parameters
    template<typename T, typename... Args>
    void runTask(T&& callable, Args&&... args)
    {
      m_tasks.push_back(std::async(std::launch::async, callable, args...));
    }

    void clearTasks();

  private:
    bool m_tasks_done = false;
    std::vector<std::future<void>> m_tasks;
  };
}
