#pragma once
#include "utilities/utils/utils.hpp"
#include <enkiTS/TaskScheduler.h>

namespace utilities
{
struct CallbackTask : enki::ITaskSet
{
  CallbackTask( std::function<void()> callback )
      : m_callback{ std::move( callback ) }
  {
    m_SetSize = 1;
  }

  void ExecuteRange( enki::TaskSetPartition range_, uint32_t threadnum_ ) override
  {
    m_callback();
  }

  std::function<void()> m_callback;
  enki::Dependency dependency;
};
} // namespace utilities