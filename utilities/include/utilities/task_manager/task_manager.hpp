#pragma once
#include "precompiled/pch.hpp"
#include "task.hpp"
#include <enkiTS/TaskScheduler.h>

namespace utilities
{

enum class TaskType
{
  Callback,
  Test
};

struct Task
{
  std::string typeId;
  std::shared_ptr<enki::ITaskSet> taskPtr;
};

class TaskManager
{
public:
  TaskManager();
  ~TaskManager();

  /**
   * @brief Get the task scheduler ref
   * @return
   */
  auto getScheduler() -> enki::TaskScheduler&;

  /**
   * @brief Add the task set to the pipe and let the task scheduler do its job
   * @param pSet Pointer to the TaskSet
   * @return
   */
  auto addTaskSetToPipe( enki::ITaskSet* pSet ) -> void;

  /**
   * @brief Update the task vector and erase already finished tasks
   * @return
   */
  auto onUpdate() -> void;

  /**
   * @brief Add a task to the vector
   * @tparam T Type of task
   * @tparam ...Args Packed params for Task ctors
   * @param ...args
   * @return Pointer to the created task
   */
  template <typename T, typename... Args>
  auto addTask( Args&&... args ) -> Task*
  {
    std::unique_ptr<Task> task;
    task = std::make_unique<Task>( Task{ .typeId = std::string{ typeid( T ).name() },
                                         .taskPtr = std::make_shared<T>( std::forward<Args>( args )... ) } );
    m_tasks.push_back( std::move( task ) );
    return m_tasks.back().get();
  }

private:
  std::vector<std::unique_ptr<Task>> m_tasks;
  enki::TaskScheduler m_taskScheduler;
  enki::TaskSchedulerConfig m_config;
};
} // namespace utilities
