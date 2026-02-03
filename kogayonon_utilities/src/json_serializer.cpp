#include "utilities/json_serializer/json_serializer.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace kogayonon_utilities
{

JsonSerializer::JsonSerializer( const std::string& path )
    : m_fileStream{ path, std::ios::out }
    , m_buffer{} //, m_os{ FileWriteStream( fopen( path.c_str(), "wb" ), m_writeBuffer, sizeof( m_writeBuffer ) ) }
                 //, m_writer{ std::make_unique<Writer<FileWriteStream>>( m_os ) }
    , m_writer{ std::make_unique<PrettyWriter<StringBuffer>>( m_buffer ) }
{
}

JsonSerializer::~JsonSerializer()
{
  m_fileStream.flush();
  if ( m_fileStream.is_open() )
    m_fileStream.close();
}

auto JsonSerializer::startDocument() -> JsonSerializer&
{
  m_writer->StartObject();
  return *this;
}

auto JsonSerializer::endDocument() -> JsonSerializer&
{
  m_writer->EndObject();
  m_fileStream << m_buffer.GetString();
  m_fileStream.flush();
  return *this;
}

auto JsonSerializer::startArray( const std::string& key ) -> JsonSerializer&
{
  if ( !key.empty() )
    m_writer->Key( key.c_str() );

  m_writer->StartArray();
  return *this;
}

auto JsonSerializer::endArray() -> JsonSerializer&
{
  m_writer->EndArray();
  return *this;
}

auto JsonSerializer::addKey( const std::string& key ) -> JsonSerializer&
{
  m_writer->Key( key.c_str() );
  return *this;
}

auto JsonSerializer::startObject( const std::string& key ) -> JsonSerializer&
{
  if ( !key.empty() )
    m_writer->Key( key.c_str() );

  m_writer->StartObject();
  return *this;
}

auto JsonSerializer::endObject() -> JsonSerializer&
{
  m_writer->EndObject();
  return *this;
}

auto JsonSerializer::saveVec3( const glm::vec3& vec ) -> JsonSerializer&
{
  startArray().addValue( vec.x ).addValue( vec.y ).addValue( vec.z ).endArray();
  return *this;
}

auto JsonSerializer::saveVec4( const glm::vec4& vec ) -> JsonSerializer&
{
  startArray().addValue( vec.x ).addValue( vec.y ).addValue( vec.z ).addValue( vec.w ).endArray();
  return *this;
}

auto JsonSerializer::saveVec3( const std::string& key, const glm::vec3& vec ) -> JsonSerializer&
{
  startArray( key ).addValue( vec.x ).addValue( vec.y ).addValue( vec.z ).endArray();
  return *this;
}

auto JsonSerializer::saveVec4( const std::string& key, const glm::vec4& vec ) -> JsonSerializer&
{
  startArray( key ).addValue( vec.x ).addValue( vec.y ).addValue( vec.z ).addValue( vec.w ).endArray();
  return *this;
}

} // namespace kogayonon_utilities
