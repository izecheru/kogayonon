#pragma once
#include <entt/entt.hpp>

namespace kogayonon_core
{
class EventEmitter
{
public:
  EventEmitter() = default;
  ~EventEmitter() = default;

  template <typename TEvent, typename Func>
  void addListener( TEvent&& event, Func&& func )
  {
    m_emitter.on<TEvent>( func );
  }

  template <typename TEvent>
  void publish( TEvent&& event )
  {
    m_emitter.publish<TEvent>( event );
  }

  template <typename TEvent>
  void removeListener()
  {
    m_emitter.erase<TEvent>();
  }

private:
  struct CustomEmitter : entt::emitter<CustomEmitter>
  {
  };

  CustomEmitter m_emitter{};
};
} // namespace kogayonon_core