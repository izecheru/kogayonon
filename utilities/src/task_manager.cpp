#include "utilities/task_manager/task_manager.hpp"

utilities::TaskManager::TaskManager()
{
    m_config.numTaskThreadsToCreate = 10;
    m_taskScheduler.Initialize( m_config );

    m_pin.taskScheduler = &m_taskScheduler;
    m_pin.threadNum = m_taskScheduler.GetNumTaskThreads() - 1;
    m_taskScheduler.AddPinnedTask( &m_pin );
}

utilities::TaskManager::~TaskManager()
{
    m_taskScheduler.WaitforAllAndShutdown();
}

auto utilities::TaskManager::getScheduler() -> enki::TaskScheduler&
{
    return m_taskScheduler;
}

auto utilities::TaskManager::addTaskSetToPipe( enki::ITaskSet* pSet ) -> void
{
    m_taskScheduler.AddTaskSetToPipe( pSet );
}

auto utilities::TaskManager::onUpdate() -> void
{
    if ( !m_tasks.empty() )
    {
        std::erase_if( m_tasks,
                       []( const std::unique_ptr<CallbackTask>& callback ) { return callback->GetIsComplete(); } );
    }

    if ( !m_pinnedTasks.empty() )
    {
        std::erase_if( m_pinnedTasks, []( const std::unique_ptr<PinnedCallbackTask>& callback ) {
            return callback->GetIsComplete();
        } );
    }
}

auto utilities::TaskManager::addPinnedTaskToExecution( enki::IPinnedTask* pTask ) -> void
{
    m_taskScheduler.AddPinnedTask( pTask );
}
