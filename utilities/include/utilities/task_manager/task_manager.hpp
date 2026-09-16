#pragma once
#include <enkiTS/TaskScheduler.h>
#include "precompiled/pch.hpp"
#include "task.hpp"

#define MAX_IO_THREADS 4

namespace utilities
{

enum class TaskType
{
  Callback,
  Test
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

  auto addPinnedTaskToExecution( enki::IPinnedTask* pTask ) -> void;

  /**
   * @brief Add a pinned task, those are primarily functions designated for vulkan resources, they have a
   * unique way of managing the thread that the task is ran on, most likely the threadId will be used for indexing
   * into command pools to generate command buffers from multiple threads
   * @tparam Fn
   * @param fn
   * @return
   */
  template <typename Fn>
    requires std::invocable<Fn>
  auto addPinnedTask( Fn&& fn ) -> PinnedCallbackTask*
  {
    auto task = std::make_unique<PinnedCallbackTask>( std::forward<Fn>( fn ) );
    task->threadNum = 1 + ( m_currentThreadNum % MAX_IO_THREADS );
    KINFO( "[TaskManager] Task will be executed on thread num {}", task->threadNum );
    m_currentThreadNum = ( m_currentThreadNum + 1 ) % MAX_IO_THREADS;
    m_pinnedTasks.emplace_back( std::move( task ) );
    return m_pinnedTasks.back().get();
  }

  /**
   * @brief Update the task vector and erase already finished tasks
   * @return
   */
  auto onUpdate() -> void;

  template <typename Fn>
    requires std::invocable<Fn>
  auto addTask( Fn&& fn, bool addToPipe = false ) -> CallbackTask*
  {
    auto task = std::make_unique<CallbackTask>( std::forward<Fn>( fn ) );
    m_tasks.push_back( std::move( task ) );

    if ( addToPipe )
    {
      m_taskScheduler.AddTaskSetToPipe( m_tasks.back().get() );
    }

    return m_tasks.back().get();
  }

  template <typename T>
  auto addDependency( T* dependent, T* predecessor ) -> void
  {
    KASSERT( dependent && predecessor );
    dependent->SetDependency( dependent->dependency, predecessor );
  }

private:
  std::vector<std::unique_ptr<CallbackTask>> m_tasks;
  std::vector<std::unique_ptr<PinnedCallbackTask>> m_pinnedTasks;
  enki::TaskScheduler m_taskScheduler;
  enki::TaskSchedulerConfig m_config;
  RunPinnedTaskLoopTask m_pin{};

  // this is like the currentFrame from swapchain
  uint32_t m_currentThreadNum{ 0u };
};
} // namespace utilities
