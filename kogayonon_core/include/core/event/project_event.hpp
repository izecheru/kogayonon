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

  inline std::filesystem::path getPath()
  {
    return m_path;
  }

private:
  std::filesystem::path m_path;
};

class ProjectSaveEvent : public IEvent
{
public:
  explicit ProjectSaveEvent( const std::filesystem::path path )
      : m_path{ path }
  {
  }

private:
  std::filesystem::path m_path;
};
} // namespace kogayonon_core