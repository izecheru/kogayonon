#include "core/task/task_manager.h"
#include "core/time_tracker/time_tracker.h"

namespace kogayonon
{
  bool TaskManager::completed()
  {
    if (m_tasks_done) return false;

    m_tasks_done = std::all_of(m_tasks.begin(), m_tasks.end(), [](std::future<void>& f)
      {
        if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
          f.get();
          return true;
        }
        return false;
      });

    if (m_tasks_done)
    {
      Logger::logInfo("Tasks done");
      clearTasks();
      return true;
    }

    return false;
  }

  void TaskManager::clearTasks()
  {
    if (m_tasks_done)
    {
      return;
    }
    m_tasks.clear();
  }
}