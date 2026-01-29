#pragma once
#include <chrono>
#include <mutex>
#include <sol/sol.hpp>
#include <unordered_map>

namespace kogayonon_utilities
{
class TimeTracker
{
private:
#pragma region USINGS
  using clock = std::chrono::steady_clock;
  using point = clock::time_point;
  using duration = std::chrono::duration<double>;
  using duration_map = std::unordered_map<std::string, std::pair<point, duration>>;
#pragma endregion

public:
  TimeTracker() = default;
  ~TimeTracker() = default;

  void update( const std::string& key );
  void start( const std::string& key );
  auto getDuration( const std::string& key ) -> duration;

  static void createLuaBindings( sol::state& lua );

private:
  std::mutex m_timeMutex;
  duration_map m_durationMap;
};
} // namespace kogayonon_utilities