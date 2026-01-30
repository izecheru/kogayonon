#include "utilities/yaml_serializer/yaml_serializer.hpp"

namespace kogayonon_utilities
{
YamlSerializer::YamlSerializer( const std::string& path )
    : m_fileStream{ path, std::ios::out }
    , m_yamlEmitter{ m_fileStream }
{
}

YamlSerializer::~YamlSerializer()
{
  if ( m_fileStream.is_open() )
    m_fileStream.close();
}
} // namespace kogayonon_utilities
