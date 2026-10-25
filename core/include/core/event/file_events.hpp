#pragma once
#include "event.hpp"

namespace core
{
enum FileEventType
{
    Create = 1 << 0,
    Delete = 1 << 1,
    Modify = 1 << 2,
    Rename = 1 << 3,
    Drop = 1 << 4
};

class FileEvent : public IEvent
{
  public:
    explicit FileEvent( const std::string& path, const std::string& name, const FileEventType type )
        : m_name{ name }
        , m_path{ path }
        , m_type{ type }
    {
    }

    ~FileEvent() = default;

    auto getName() const -> std::string
    {
        return m_name;
    }

    auto getPath() const -> std::string
    {
        return m_path;
    }

    auto getType() const -> FileEventType
    {
        return m_type;
    }

  protected:
    std::string m_name;
    std::string m_path;
    FileEventType m_type;
};
} // namespace core