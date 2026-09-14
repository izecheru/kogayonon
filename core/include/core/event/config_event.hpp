#pragma once
#include "core/event/event.hpp"

namespace core
{
class ConfigChangedEvent : public IEvent
{
public:
  ConfigChangedEvent() = default;
  ~ConfigChangedEvent() = default;

private:
};
} // namespace core