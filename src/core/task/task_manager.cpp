#include "core/task/task_manager.h"

namespace kogayonon
{
  bool TaskManager::completed()
  {
    return m_tasks_done;
  }

  void TaskManager::executeTasks()
  {
    if (!m_tasks.empty())
    {
      if (m_tasks.front().wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
      {
        m_tasks.pop();
        Logger::logInfo("Task done, onto the next one");
      }
    }
    m_tasks_done = true;
  }
}