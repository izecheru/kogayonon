#pragma once
#include "events/event.h"

namespace kogayonon
{
  class Layer
  {
  public:
    Layer() = default;
    virtual ~Layer() {};
    virtual bool onEvent(Event& event) = 0;
    virtual void onRender() = 0;
    virtual void onUpdate() = 0;
  };
}

