#pragma once
#include <functional>
#include <map>

#include "core/singleton/singleton.h"
#include "event.h"
namespace kogayonon
{
  class EventListener : public Singleton<EventListener>
  {
  public:
    EventListener()     = default;
    using EventCallback = std::function<bool(Event&)>;

    template <typename T>

    // probably should make this reference instead of copying all that shit
    void addCallback(EventCallback callback)
    {
      auto type = T::getStaticType();
      m_callbacks[type].push_back({0, callback});
    }

    template <typename T>
    void removeCallback(char callback_id)
    {
      auto type = T::getEventType();
      auto& vec = m_callbacks[type];

      vec.erase(std::remove_if(vec.begin(), vec.end(), [callback_id](const auto& pair) { return pair.first == callback_id; }), vec.end());
    }

    void dispatch(Event& event)
    {
      auto it = m_callbacks.find(event.getEventType());
      if (it != m_callbacks.end())
      {
        for (const auto& [id, callback] : it->second)
        {
          event.m_handled |= callback(event);
        }
      }
    }

  private:
    // TODO rethink event system so that i can easely subscribe and unsubscribe the correct way and at
    // the correct time from a specific event inside a specific class listener
    std::unordered_map<EventType, std::vector<std::pair<unsigned char, EventCallback>>> m_callbacks;
  };
} // namespace kogayonon
