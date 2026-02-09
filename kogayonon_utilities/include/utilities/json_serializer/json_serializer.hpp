#pragma once
#include <fstream>
#include <glm/glm.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <string>

using namespace rapidjson;

namespace kogayonon_utilities
{
static auto getVec3( const Value& v ) -> glm::vec3
{
  if ( !v.IsArray() || v.Size() != 3 )
    throw std::runtime_error( "Expected array with size 3" );

  return glm::vec3{ v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat() };
}

static auto getVec4( const Value& v ) -> glm::vec4
{
  if ( !v.IsArray() || v.Size() != 4 )
    throw std::runtime_error( "Expected array with size 4" );

  return glm::vec4{ v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat(), v[3].GetFloat() };
}

class JsonSerializer
{
public:
  JsonSerializer( const std::string& path );
  ~JsonSerializer();

  auto startDocument() -> JsonSerializer&;
  auto startArray( const std::string& key = "" ) -> JsonSerializer&;
  auto endDocument() -> JsonSerializer&;
  auto endArray() -> JsonSerializer&;

  auto startObject( const std::string& key = "" ) -> JsonSerializer&;
  auto addKey( const std::string& key ) -> JsonSerializer&;
  auto endObject() -> JsonSerializer&;

  auto saveVec3( const glm::vec3& vec ) -> JsonSerializer&;
  auto saveVec4( const glm::vec4& vec ) -> JsonSerializer&;

  auto saveVec3( const std::string& key, const glm::vec3& vec ) -> JsonSerializer&;
  auto saveVec4( const std::string& key, const glm::vec4& vec ) -> JsonSerializer&;

  template <typename T>
  auto addValue( const T& value ) -> JsonSerializer&
  {
    if constexpr ( std::is_same<T, int>::value )
    {
      m_writer->Int( value );
    }

    if constexpr ( std::is_same<T, float>::value )
    {
      m_writer->Double( value );
    }

    return *this;
  }

  template <typename T>
  auto addKeyValuePair( const std::string& key, const T& value ) -> JsonSerializer&
  {
    if ( !key.empty() )
      m_writer->Key( key.c_str() );

    if constexpr ( std::is_same<T, int>::value )
    {
      m_writer->Int( value );
    }
    else if constexpr ( std::is_same<T, uint32_t>::value )
    {
      m_writer->Uint( value );
    }
    else if constexpr ( std::is_same<T, float>::value )
    {
      m_writer->Double( value );
    }
    else if constexpr ( std::is_same<T, std::string>::value )
    {
      m_writer->String( value.c_str() );
    }
    else if constexpr ( std::is_same<T, glm::vec3>::value )
    {
      return saveVec3( value );
    }
    else if constexpr ( std::is_same<T, glm::vec4>::value )
    {
      return saveVec4( value );
    }
    else
    {
      assert( true && "Type is not handled in addKeyValuePair" && __FILE__ );
    }

    return *this;
  }

private:
  std::fstream m_fileStream;
  StringBuffer m_buffer;
  std::unique_ptr<PrettyWriter<StringBuffer>> m_writer;
  // FileWriteStream m_os;
  // char m_writeBuffer[65536];
  // std::unique_ptr<Writer<FileWriteStream>> m_writer;
};
} // namespace kogayonon_utilities
