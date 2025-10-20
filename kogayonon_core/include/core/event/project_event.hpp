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

  inline std::filesystem::path getPath() const
  {
    return m_path;
  }

private:
  std::filesystem::path m_path;
  EventType m_type{ EventType::ProjectLoad };
};

class ProjectSaveEvent : public IEvent
{
public:
  explicit ProjectSaveEvent( const std::filesystem::path path )
      : m_path{ path }
  {
  }

  inline std::filesystem::path getPath() const
  {
    return m_path;
  }

private:
  std::filesystem::path m_path;
  EventType m_type{ EventType::ProjectSave };
};

class ProjectCreateEvent : public IEvent
{
public:
  explicit ProjectCreateEvent( const std::filesystem::path path )
      : m_path{ path }
  {
  }

  inline std::filesystem::path getPath() const
  {
    return m_path;
  }

private:
  std::filesystem::path m_path;
  EventType m_type{ EventType::ProjectCreate };
};
} // namespace kogayonon_core