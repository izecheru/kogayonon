#include "mouse_events.h"
#include <iostream>
#include <sstream>

using std::cout;

namespace kogayonon {

  float MouseMoved::GetY() {
    return m_MouseY;
  }

  float MouseMoved::GetX() {
    return m_MouseX;
  }

  std::string MouseMoved::toString() {
    std::stringstream ss{};
    ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
    std::string result = ss.str();
    return result;
  }
}
