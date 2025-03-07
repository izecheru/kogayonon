#include "core/task/task_manager.h"
#include "core/time_tracker/time_tracker.h"
#include "core/logger.h"

namespace kogayonon
{
  TaskManager::TaskManager(size_t thread_count)
  {
    for(size_t i = 0; i < thread_count; ++i)
    {
      m_workers.emplace_back([this]
        {
          workerThread();
        });
    }
  }

  TaskManager::~TaskManager()
  {
    stop();
  }

  void TaskManager::enqueue(std::function<void()> task)
  {
    {
      std::lock_guard lock(m_queue_mutex);
      m_tasks.push(std::move(task));
    }
    m_cvar.notify_one();
  }

  void TaskManager::workerThread()
  {
    while(true)
    {
      std::function<void()> task;
      {
        std::unique_lock lock(m_queue_mutex);

        // this makes the thread resume waiting if the conditions are not satisfied
        // this only works with std::unique_lock

        m_cvar.wait(lock, [this]
          {
            return m_stop || !m_tasks.empty();
          });

        //

        if(m_stop && m_tasks.empty())
        {
          break;
        }

        task = std::move(m_tasks.front());
        m_tasks.pop();
      }
      try
      {
        task();  // Execute task
      }
      catch(const std::exception& e)
      {
        // Log the error instead of crashing the thread
        Logger::logError("Task execution error: ", e.what());
      }
    }
  }

  void TaskManager::stop()
  {
    {
      std::lock_guard lock(m_queue_mutex);
      m_stop = true;
    }
    m_cvar.notify_all();  // Wake all worker threads

    for(std::thread& worker : m_workers)
    {
      if(worker.joinable())
      {
        worker.join();
      }
    }
  }
}