#pragma once
#include "event.hpp"

namespace core
{

enum class ImGuiWindowSource
{
  Viewport
};

class ImGuiWindowResizeEvent : public IEvent
{
public:
  ImGuiWindowResizeEvent( const ImGuiWindowSource source, uint32_t width, uint32_t height )
      : m_source{ source }
      , m_width{ width }
      , m_height{ height }
  {
  }

  auto getSource() const -> const ImGuiWindowSource
  {
    return m_source;
  }

  auto getWidth() const -> uint32_t
  {
    return m_width;
  }

  auto getHeight() const -> uint32_t
  {
    return m_height;
  }

private:
  ImGuiWindowSource m_source;
  uint32_t m_width;
  uint32_t m_height;
};

} // namespace core