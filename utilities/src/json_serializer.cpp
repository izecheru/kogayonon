#include "utilities/json_serializer/json_serializer.hpp"

namespace fs = std::filesystem;

auto utilities::JsonSerializer::getVec3( const Value& v ) -> glm::vec3
{
    if ( !v.IsArray() || v.Size() != 3 )
        throw std::runtime_error( "Expected array with size 3" );

    return glm::vec3{ v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat() };
}

auto utilities::JsonSerializer::getVec4( const Value& v ) -> glm::vec4
{
    if ( !v.IsArray() || v.Size() != 4 )
        throw std::runtime_error( "Expected array with size 4" );

    return glm::vec4{ v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat(), v[3].GetFloat() };
}

utilities::JsonSerializer::JsonSerializer( const std::string& path )
    : m_fileStream{ path, std::ios::out }
    , m_buffer{}
    , m_writer{ std::make_unique<PrettyWriter<StringBuffer>>( m_buffer ) }
{
}

utilities::JsonSerializer::~JsonSerializer()
{
    m_fileStream.flush();
    if ( m_fileStream.is_open() )
        m_fileStream.close();
}

auto utilities::JsonSerializer::startDocument() -> JsonSerializer&
{
    m_writer->StartObject();
    return *this;
}

auto utilities::JsonSerializer::endDocument() -> JsonSerializer&
{
    m_writer->EndObject();
    m_fileStream << m_buffer.GetString();
    m_fileStream.flush();
    return *this;
}

auto utilities::JsonSerializer::startArray( const std::string& key ) -> JsonSerializer&
{
    if ( !key.empty() )
        m_writer->Key( key.c_str() );

    m_writer->StartArray();
    return *this;
}

auto utilities::JsonSerializer::endArray() -> JsonSerializer&
{
    m_writer->EndArray();
    return *this;
}

auto utilities::JsonSerializer::addKey( const std::string& key ) -> JsonSerializer&
{
    m_writer->Key( key.c_str() );
    return *this;
}

auto utilities::JsonSerializer::startObject( const std::string& key ) -> JsonSerializer&
{
    if ( !key.empty() )
        m_writer->Key( key.c_str() );

    m_writer->StartObject();
    return *this;
}

auto utilities::JsonSerializer::endObject() -> JsonSerializer&
{
    m_writer->EndObject();
    return *this;
}

auto utilities::JsonSerializer::saveVec3( const glm::vec3& vec ) -> JsonSerializer&
{
    startArray().addValue( vec.x ).addValue( vec.y ).addValue( vec.z ).endArray();
    return *this;
}

auto utilities::JsonSerializer::saveVec4( const glm::vec4& vec ) -> JsonSerializer&
{
    startArray().addValue( vec.x ).addValue( vec.y ).addValue( vec.z ).addValue( vec.w ).endArray();
    return *this;
}

auto utilities::JsonSerializer::saveVec3( const std::string& key, const glm::vec3& vec ) -> JsonSerializer&
{
    startArray( key ).addValue( vec.x ).addValue( vec.y ).addValue( vec.z ).endArray();
    return *this;
}

auto utilities::JsonSerializer::saveVec4( const std::string& key, const glm::vec4& vec ) -> JsonSerializer&
{
    startArray( key ).addValue( vec.x ).addValue( vec.y ).addValue( vec.z ).addValue( vec.w ).endArray();
    return *this;
}
