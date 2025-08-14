#pragma once
#include <unordered_map>
#include <mutex>
#include "core/singleton/singleton.h"

namespace kogayonon
{
  class TimeTracker :public Singleton<TimeTracker>
  {
  private:
    using clock = std::chrono::steady_clock;
    using point = clock::time_point;
    using duration = std::chrono::duration<double>;
    using duration_map = std::unordered_map<std::string, std::pair<point, point>>;

  public:
    inline void startCount(const std::string& reason)
    {
      std::scoped_lock<std::mutex> lock(m_mutex);
      m_duration_map[reason].first = clock::now();
    }

    inline void stopCount(const std::string& reason)
    {
      std::scoped_lock<std::mutex> lock(m_mutex);
      m_duration_map[reason].second = clock::now();
    }

    duration getDuration(const std::string& reason) const;

    inline void setDelta(float d)
    {
      delta_time = d;
    }
    inline double getDelta()const
    {
      return delta_time;
    }

  private:
    mutable std::mutex m_mutex{};
    duration_map m_duration_map{};
    float delta_time = 0;
  };
}
