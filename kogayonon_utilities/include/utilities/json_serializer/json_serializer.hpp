#pragma once
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <string>

using namespace rapidjson;

namespace kogayonon_utilities
{

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

    if constexpr ( std::is_same<T, float>::value )
    {
      m_writer->Double( value );
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
