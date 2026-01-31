#include "utilities/yaml_serializer/yaml_serializer.hpp"
#include <glm/glm.hpp>

namespace kogayonon_utilities
{
YamlSerializer::YamlSerializer( const std::string& path )
    : m_fileStream{ path, std::ios::out | std::ios::trunc }
    , m_yamlEmitter{ m_fileStream }
{
}

YamlSerializer::YamlSerializer()
    : m_yamlEmitter{}
{
}

YamlSerializer::~YamlSerializer()
{
  if ( m_fileStream.is_open() )
    m_fileStream.close();
}

void YamlSerializer::initFstream( const std::string& path )
{
  m_fileStream = std::fstream{ path, std::ios::out | std::ios::trunc };
}

void YamlSerializer::writeToFile()
{
  m_fileStream << m_yamlEmitter.c_str();
  m_fileStream.flush();
}
} // namespace kogayonon_utilities
