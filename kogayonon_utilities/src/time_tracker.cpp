#include "utilities/time_tracker/time_tracker.h"
#include "logger/logger.h"
using namespace kogayonon_logger;

namespace kogayonon_utilities
{
void TimeTracker::update(const std::string& key)
{
    std::lock_guard lock(m_timeMutex);
    auto it = m_durationMap.find(key);
    auto now = std::chrono::high_resolution_clock::now();
    if (it != m_durationMap.end())
    {
        it->second.second = now - it->second.first;
        it->second.first = now;
    }
}

void TimeTracker::start(const std::string& key)
{
    std::lock_guard lock(m_timeMutex);
    auto now = std::chrono::high_resolution_clock::now();
    m_durationMap.emplace(key, std::make_pair(now, duration(0.0)));
}

TimeTracker::duration TimeTracker::getDuration(const std::string& key)
{
    std::lock_guard lock(m_timeMutex);
    auto it = m_durationMap.find(key);
    if (it != m_durationMap.end())
    {
        return it->second.second;
    }
    Logger::critical("Duration for ", key, " was not found!");
    return duration(0);
}
} // namespace kogayonon_utilities