#include <cassert>
#include <chrono>
#include "core/time_tracker/time_tracker.h"

namespace kogayonon
{

  Timer::duration Timer::getDuration(const std::string& reason) const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_duration_map.find(reason);
    if (it != m_duration_map.end())
    {
      return it->second.second - it->second.first;
    }
    return duration(0.0f);
  }
}
