#include "core/task/task_manager.h"
#include "core/time_tracker/time_tracker.h"

namespace kogayonon
{

  TaskManager::TaskManager(size_t threadCount)
  {
    for(size_t i = 0; i < threadCount; ++i)
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
      std::unique_lock lock(m_queue_mutex);
      m_tasks.push(std::move(task));
    }
    m_cvar.notify_one();  // Wake up a worker thread
  }

  void TaskManager::workerThread()
  {
    while(true)
    {
      std::function<void()> task;
      {
        std::unique_lock lock(m_queue_mutex);
        m_cvar.wait(lock, [this]
          {
            return m_stop || !m_tasks.empty();
          });

        if(m_stop && m_tasks.empty())
        {
          return;
        }

        task = std::move(m_tasks.front());
        m_tasks.pop();
      }
      task();  // Execute task
    }
  }

  void TaskManager::stop()
  {
    {
      std::unique_lock lock(m_queue_mutex);
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
