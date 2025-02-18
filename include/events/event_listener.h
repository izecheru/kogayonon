#pragma once
#include <functional>
#include "core/singleton/singleton.h"
#include "event.h"
#include <map>
namespace kogayonon
{
  class EventListener :public Singleton<EventListener> {
  public:
    EventListener() {}
    using EventCallback = std::function<bool(Event&)>;

    template<typename T>
    void addCallback(EventCallback callback) {
      auto type = T::getStaticType();
      char callback_id = m_id++;
      m_callbacks[type].push_back({ callback_id,callback });
    }

    template<typename T>
    void removeCallback(char callback_id) {
      auto type = T::getEventType();
      auto& vec = m_callbacks[type];

      vec.erase(std::remove_if(vec.begin(), vec.end(),
        [callback_id](const auto& pair)
        {
          return pair.first == callback_id;
        }), vec.end());
    }

    void dispatch(Event& event) {
      auto it = m_callbacks.find(event.getEventType());
      if (it != m_callbacks.end()) {
        for (const auto& [id, callback] : it->second) {
          event.m_handled |= callback(event);
        }
      }
    }

  private:
    std::unordered_map <EventType, std::vector<std::pair<unsigned char, EventCallback>>> m_callbacks;
    // don't think we need more than 255 ids
    unsigned char m_id = 0;
  };
}
