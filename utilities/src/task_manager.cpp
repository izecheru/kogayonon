#include "utilities/task_manager/task_manager.hpp"

utilities::TaskManager::TaskManager()
{
  m_taskScheduler.Initialize( m_config );
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
  if ( m_tasks.empty() )
  {
    return;
  }

  for ( auto it = m_tasks.begin(); it != m_tasks.end(); )
  {
    if ( it->get()->taskPtr->GetIsComplete() )
    {
      it = m_tasks.erase( it );
    }
    else
    {
      ++it;
    }
  }
}
