#pragma once
#include <glm/glm.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include "precompiled/pch.hpp"

using namespace rapidjson;

namespace utilities
{

class JsonSerializer
{
  public:
    explicit JsonSerializer( const std::string& path );
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

    auto getVec3( const Value& v ) -> glm::vec3;
    auto getVec4( const Value& v ) -> glm::vec4;

    template <typename T>
    auto addValue( const T& value ) -> JsonSerializer&;

    template <typename T>
    auto addKeyValuePair( const std::string& key, const T& value ) -> JsonSerializer&;

  private:
    std::fstream m_fileStream;
    StringBuffer m_buffer;
    std::unique_ptr<PrettyWriter<StringBuffer>> m_writer;
    // FileWriteStream m_os;
    // char m_writeBuffer[65536];
    // std::unique_ptr<Writer<FileWriteStream>> m_writer;
};

#include "utilities/json_serializer/json_serializer.inl"
} // namespace utilities
