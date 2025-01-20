#include "mouse_events.h"
#include <iostream>
#include <sstream>

using std::cout;

namespace kogayonon
{

  float MouseMovedEvent::getY()
  {
    return m_MouseY;
  }

  float MouseMovedEvent::getX()
  {
    return m_MouseX;
  }

  std::string MouseMovedEvent::toString() const
  {
    std::stringstream ss{};
    ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
    std::string result = ss.str();
    return result;
  }

  float MouseClickedEvent::getX()
  {
    return m_xpos;
  }

  float MouseClickedEvent::getY()
  {
    return m_ypos;
  }

  std::string MouseClickedEvent::toString() const
  {
    std::stringstream ss{};
    ss << "MouseClickedEvent: " << m_xpos << ", " << m_ypos;
    std::string result = ss.str();
    return result;
  }

}
