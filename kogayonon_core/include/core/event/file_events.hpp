#pragma once

#include <filesystem>
#include "event.hpp"

namespace kogayonon_core
{
class FileCreatedEvent : public IEvent
{
public:
  explicit FileCreatedEvent( const std::string& path, const std::string& name )
      : m_name( name )
      , m_path( path )
  {
  }

  ~FileCreatedEvent() = default;

  inline std::string getName() const
  {
    return m_name;
  }

  inline std::string getPath() const
  {
    return m_path;
  }

  EVENT_CLASS_TYPE( FileCreated )

protected:
  std::string m_name;
  std::string m_path;
};

class FileRenamedEvent : public IEvent
{
public:
  explicit FileRenamedEvent( const std::string& path, const std::string& oldName, const std::string& newName )
      : m_name( newName )
      , m_oldName( oldName )
      , m_path( path )
  {
  }

  inline std::string getName() const
  {
    return m_name;
  }

  inline std::string getPath() const
  {
    return m_path;
  }

  ~FileRenamedEvent() = default;

  EVENT_CLASS_TYPE( FileRenamed )

protected:
  std::string m_oldName;
  std::string m_name;
  std::string m_path;
};

class FileDeletedEvent : public IEvent
{
public:
  explicit FileDeletedEvent( const std::string& path, const std::string name )
      : m_path( path )
      , m_name( name )
  {
  }

  ~FileDeletedEvent() = default;

  inline std::string getPath() const
  {
    return m_path;
  }

  inline std::string getName() const
  {
    return m_name;
  }

  EVENT_CLASS_TYPE( FileDeleted )

protected:
  std::string m_name;
  std::string m_path;
};

class FileModifiedEvent : public IEvent
{
public:
  explicit FileModifiedEvent( const std::string& path, const std::string name )
      : m_path( path )
      , m_name( name )
  {
  }

  ~FileModifiedEvent() = default;

  inline std::string getPath() const

  {
    return m_path;
  }

  inline std::string getName() const
  {
    return m_name;
  }

  EVENT_CLASS_TYPE( FileModified )

protected:
  std::string m_name;
  std::string m_path;
};
} // namespace kogayonon_core