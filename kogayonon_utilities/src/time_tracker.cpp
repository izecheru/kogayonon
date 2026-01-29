#include "utilities/time_tracker/time_tracker.hpp"
#include <spdlog/spdlog.h>

namespace kogayonon_utilities
{
void TimeTracker::update( const std::string& key )
{
  std::lock_guard lock( m_timeMutex );
  auto it = m_durationMap.find( key );
  auto now = std::chrono::high_resolution_clock::now();
  if ( it != m_durationMap.end() )
  {
    it->second.second = now - it->second.first;
    it->second.first = now;
  }
}

void TimeTracker::start( const std::string& key )
{
  std::lock_guard lock( m_timeMutex );
  auto now = std::chrono::high_resolution_clock::now();
  m_durationMap.emplace( key, std::make_pair( now, duration( 0.0 ) ) );
}

auto TimeTracker::getDuration( const std::string& key ) -> TimeTracker::duration
{
  std::lock_guard lock( m_timeMutex );
  auto it = m_durationMap.find( key );
  if ( it != m_durationMap.end() )
  {
    return it->second.second;
  }
  spdlog::critical( "Duration for {} was not found ", key );
  return duration{ 0 };
}

void TimeTracker::createLuaBindings( sol::state& lua )
{
  lua.new_usertype<TimeTracker>(
    "TimeTracker",
    "update",
    []( TimeTracker& self, const std::string& key ) { self.update( key ); },
    "start",
    []( TimeTracker& self, const std::string& key ) { self.start( key ); },
    "getDuration",
    []( TimeTracker& self, const std::string& key, sol::this_state currentState ) -> sol::object {
      return sol::make_object( currentState, self.getDuration( key ).count() );
    } );
}
} // namespace kogayonon_utilities