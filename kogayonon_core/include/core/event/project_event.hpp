#pragma once
#include <filesystem>
#include "core/event/event.hpp"

namespace kogayonon_core
{
class ProjectLoadEvent : public IEvent
{
public:
  explicit ProjectLoadEvent( const std::filesystem::path path )
      : m_path{ path }
  {
  }

private:
  std::filesystem::path m_path;
};
} // namespace kogayonon_core