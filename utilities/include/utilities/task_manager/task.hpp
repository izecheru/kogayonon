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

template <typename TData>
struct DataTaskSet : enki::ITaskSet
{
    explicit DataTaskSet( TData&& data, std::function<void( TData& )>&& callback )
        : fn{ std::move( callback ) }
        , container{ std::move( data ) }
    {
        m_SetSize = 1;
    }

    void ExecuteRange( enki::TaskSetPartition range_, uint32_t threadnum_ ) override
    {
        fn( container );
    }

    TData container;
    std::function<void( TData& )> fn;
    enki::Dependency dependency;
};

struct TaskSet : enki::ITaskSet
{
    explicit TaskSet( std::function<void()>&& callback )
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

struct PinnedTask : enki::IPinnedTask
{
    explicit PinnedTask( std::function<void( uint32_t )>&& func_ )
        : func{ std::move( func_ ) }
    {
    }

    void Execute() override
    {
        func( threadNum );
    }

    std::function<void( uint32_t )> func;
    enki::Dependency dependency;
};

template <typename TData>
struct PinnedDataTask : enki::IPinnedTask
{
    explicit PinnedDataTask( TData&& data_, std::function<void( TData& )>&& func_ )
        : func{ std::move( func_ ) }
        , data{ std::move( data_ ) }
    {
    }

    void Execute() override
    {
        func( data );
    }

    TData data;
    std::function<void( TData& )> func;
    enki::Dependency dependency;
};

} // namespace utilities