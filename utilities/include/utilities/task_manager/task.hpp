#pragma once
#include "utilities/utils/utils.hpp"
#include <enkiTS/TaskScheduler.h>

namespace utilities
{

/**
 * @brief This sits and listens for a new pinned task and then executes it
 */
struct RunPinnedTaskLoopTask : enki::IPinnedTask
{
    void Execute() override
    {
        while ( !taskScheduler->GetIsShutdownRequested() )
        {
            taskScheduler->WaitForNewPinnedTasks();
            taskScheduler->RunPinnedTasks();
        }
    }

    enki::TaskScheduler* taskScheduler;
};

struct CallbackTask : enki::ITaskSet
{
    explicit CallbackTask( const std::function<void()>&& callback )
        : fn{ std::move( callback ) }
    {
        m_SetSize = 1;
    }

    void ExecuteRange( enki::TaskSetPartition range_, uint32_t threadnum_ ) override
    {
        fn();
    }

    std::function<void()> fn;
    enki::Dependency dependency;
};

struct PinnedCallbackTask : enki::IPinnedTask
{
    explicit PinnedCallbackTask( std::function<void()>&& func_ )
        : func{ std::move( func_ ) }
    {
    }

    void Execute() override
    {
        func();
    }

    std::function<void()> func;
    enki::Dependency dependency;
};

} // namespace utilities