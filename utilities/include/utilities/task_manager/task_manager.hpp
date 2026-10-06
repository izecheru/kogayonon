#pragma once
#include <enkiTS/TaskScheduler.h>
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
    auto addTaskSetToPipe( enki::ITaskSet* pSet, const std::string& name = "" ) -> void;

    auto eraseTask( enki::ITaskSet* task ) -> void;

    auto addPinnedTaskToExecution( enki::IPinnedTask* pTask ) -> void;

    template <class TData, typename Fn>
        requires std::invocable<Fn, TData&>
    auto addTask( TData&& data, Fn&& fn, bool addToPipe = false ) -> DataTaskSet<TData>*;

    template <typename Fn>
        requires std::invocable<Fn>
    auto addTask( Fn&& fn, bool addToPipe = false ) -> TaskSet*;

    template <typename Fn>
        requires std::invocable<Fn>
    auto addPinnedTask( Fn&& fn, bool executeImediately = false ) -> PinnedTask*;

    template <class TData, typename Fn>
        requires std::invocable<Fn, TData&>
    auto addPinnedDataTask( TData&& data, Fn&& fn, bool executeImediately = false ) -> PinnedDataTask<TData>*;

    auto getCurrentThreadNum() -> uint32_t;

  private:
    std::vector<std::unique_ptr<enki::ITaskSet>> m_tasks;
    std::vector<std::unique_ptr<enki::IPinnedTask>> m_pinnedTasks;
    enki::TaskScheduler m_taskScheduler;
    enki::TaskSchedulerConfig m_config;
    RunPinnedTaskLoopTask m_pin{};

    uint32_t m_currentThreadNum{ 0u };
    std::mutex m_mutex;
};

#include "utilities/task_manager/task_manager.inl"
} // namespace utilities
