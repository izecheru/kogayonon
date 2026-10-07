#include "utilities/task_manager/task_manager.hpp"

template <typename Fn>
    requires std::invocable<Fn>
auto utilities::TaskManager::addPinnedTask( Fn&& fn, bool executeImediately ) -> utilities::PinnedTask*
{
    auto task = std::make_unique<PinnedTask>( std::forward<Fn>( fn ) );

    m_currentThreadNum = 1 + ( m_currentThreadNum + 1 ) % MAX_IO_THREADS;

    task->threadNum = m_currentThreadNum;

    m_registry.pinnedTasks.emplace_back( std::move( task ) );

    PinnedTask* taskPtr = static_cast<PinnedTask*>( m_registry.pinnedTasks.back().get() );

    if ( executeImediately )
    {
        m_taskScheduler.AddPinnedTask( taskPtr );
    }

    return taskPtr;
}

template <class TData, typename Fn>
    requires std::invocable<Fn, TData&>
auto utilities::TaskManager::addPinnedDataTask( TData&& data, Fn&& fn, bool executeImediately )
    -> utilities::PinnedDataTask<TData>*
{
    auto task = std::make_unique<PinnedDataTask<TData>>( std::forward<TData>( data ), std::forward<Fn>( fn ) );

    m_currentThreadNum = 1 + ( m_currentThreadNum + 1 ) % MAX_IO_THREADS;

    task->threadNum = m_currentThreadNum;

    m_registry.pinnedDataTasks.emplace_back( std::move( task ) );

    PinnedDataTask<TData>* taskPtr = static_cast<PinnedDataTask<TData>*>( m_registry.pinnedDataTasks.back().get() );

    if ( executeImediately )
    {
        m_taskScheduler.AddPinnedTask( taskPtr );
    }

    return taskPtr;
}

template <class TData, typename Fn>
    requires std::invocable<Fn, TData&>
auto utilities::TaskManager::addTask( TData&& data, Fn&& fn, bool addToPipe ) -> utilities::DataTaskSet<TData>*
{
    auto task = std::make_unique<DataTaskSet<TData>>( std::forward<TData>( data ), std::forward<Fn>( fn ) );
    m_registry.dataTasks.emplace_back( std::move( task ) );
    DataTaskSet<TData>* taskPtr = static_cast<DataTaskSet<TData>*>( m_registry.dataTasks.back().get() );

    if ( addToPipe )
    {
        m_taskScheduler.AddTaskSetToPipe( taskPtr );
    }

    return taskPtr;
}

template <typename Fn>
    requires std::invocable<Fn>
auto utilities::TaskManager::addTask( Fn&& fn, bool addToPipe ) -> utilities::TaskSet*
{
    auto task = std::make_unique<TaskSet>( std::forward<Fn>( fn ) );
    m_registry.tasks.emplace_back( std::move( task ) );
    TaskSet* taskPtr = static_cast<TaskSet*>( m_registry.tasks.back().get() );

    if ( addToPipe )
    {
        m_taskScheduler.AddTaskSetToPipe( taskPtr );
    }

    return taskPtr;
}
