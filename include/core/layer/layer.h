#pragma once
#include "events/event.h"

namespace kogayonon
{
  class Layer
  {
  public:
    Layer() = default;
    virtual ~Layer() {};
    virtual void render() = 0;
    virtual void onUpdate() = 0;

    void setVisible(bool state) { m_visible = state; }
    bool isVisible() const { return m_visible; }

  protected:
    bool m_visible = true;
  };
}
