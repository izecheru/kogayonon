#pragma once
#include <unordered_map>
#include <mutex>
#include "core/singleton/singleton.h"

namespace kogayonon
{
  class Timer :public Singleton<Timer>
  {
  private:
    using clock = std::chrono::steady_clock;
    using point = clock::time_point;
    using duration = std::chrono::duration<double>;
    using duration_map = std::unordered_map<std::string, std::pair<point, point>>;

  public:
    inline void startCount(const std::string& reason)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_duration_map[reason].first = clock::now();
    }

    inline void stopCount(const std::string& reason)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_duration_map[reason].second = clock::now();
    }

    duration getDuration(const std::string& reason) const;

  private:
    mutable std::mutex m_mutex{};
    duration_map m_duration_map{};
  };
}
