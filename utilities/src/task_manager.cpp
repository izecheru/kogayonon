#include "utilities/task_manager/task_manager.hpp"

utilities::TaskManager::TaskManager()
{
    m_config.numTaskThreadsToCreate = 10;
    m_taskScheduler.Initialize( m_config );

    m_pin.taskScheduler = &m_taskScheduler;
    m_pin.threadNum = m_taskScheduler.GetNumTaskThreads() - 1;
    m_taskScheduler.AddPinnedTask( &m_pin );
    m_currentThreadNum = 1;
}

utilities::TaskManager::~TaskManager()
{
    m_taskScheduler.WaitforAllAndShutdown();
}

auto utilities::TaskManager::getScheduler() -> enki::TaskScheduler&
{
    return m_taskScheduler;
}

auto utilities::TaskManager::addTaskSetToPipe( enki::ITaskSet* pSet, const std::string& name ) -> void
{
    m_taskScheduler.AddTaskSetToPipe( pSet );

    if ( !name.empty() )
    {
        KWARN( "Task {} is about to be executed", name );
    }
}

auto utilities::TaskManager::addPinnedTaskToExecution( enki::IPinnedTask* pTask ) -> void
{
    m_taskScheduler.AddPinnedTask( pTask );
}

auto utilities::TaskManager::getCurrentThreadNum() -> uint32_t
{
    std::lock_guard lock{ m_mutex };
    return m_currentThreadNum;
}

auto utilities::TaskManager::erasePinnedTask( enki::IPinnedTask* task, TaskType type ) -> void
{
    if ( !task->GetIsComplete() )
    {
        throw std::runtime_error( "TASK was not done, be careful" );
    }

    if ( type == TaskType::Data )
    {
        std::erase_if( m_registry.pinnedDataTasks, [task]( const std::unique_ptr<enki::IPinnedTask>& pinnedTask ) {
            return pinnedTask.get() == task;
        } );

        return;
    }

    std::erase_if( m_registry.pinnedTasks, [task]( const std::unique_ptr<enki::IPinnedTask>& pinnedTask ) {
        return pinnedTask.get() == task;
    } );
}

auto utilities::TaskManager::eraseTask( enki::ITaskSet* task, TaskType type ) -> void
{
    if ( !task->GetIsComplete() )
    {
        throw std::runtime_error( "TASK was not done, be careful" );
    }

    if ( type == TaskType::Data )
    {
        std::erase_if( m_registry.dataTasks,
                       [task]( const std::unique_ptr<enki::ITaskSet>& taskSet ) { return taskSet.get() == task; } );
        return;
    }

    std::erase_if( m_registry.tasks,
                   [task]( const std::unique_ptr<enki::ITaskSet>& taskSet ) { return taskSet.get() == task; } );
}

auto utilities::TaskManager::onUpdate() -> void
{
    // currently only those manage to fit into the destroy whenever criteria, the data carrying tasks certainly not
    std::erase_if( m_registry.tasks,
                   []( const std::unique_ptr<enki::ITaskSet>& taskSet ) { return taskSet->GetIsComplete(); } );
}
