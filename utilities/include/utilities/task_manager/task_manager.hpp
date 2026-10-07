#pragma once
#include <enkiTS/TaskScheduler.h>
#include "utilities/task_manager/task_registry.hpp"
#include "task.hpp"

#define MAX_IO_THREADS 4

namespace utilities
{
enum TaskType : bool
{
    None = false,
    Data = true
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

    auto eraseTask( enki::ITaskSet* task, TaskType type ) -> void;
    auto erasePinnedTask( enki::IPinnedTask* task, TaskType type ) -> void;

    auto onUpdate() -> void;
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
    enki::TaskScheduler m_taskScheduler;
    enki::TaskSchedulerConfig m_config;
    RunPinnedTaskLoopTask m_pin{};
    TaskRegistry m_registry;
    uint32_t m_currentThreadNum{ 0u };
    std::mutex m_mutex;
};

#include "utilities/task_manager/task_manager.inl"
} // namespace utilities
