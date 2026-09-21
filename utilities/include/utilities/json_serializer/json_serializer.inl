#include "utilities/json_serializer/json_serializer.hpp"
#include "utilities/utils/utils.hpp"

template <typename T>
auto utilities::JsonSerializer::addValue( const T& value ) -> JsonSerializer&
{
    if constexpr ( std::is_same<T, int>::value )
    {
        m_writer->Int( value );
    }
    else if constexpr ( std::is_same<T, uint32_t>::value )
    {
        m_writer->Uint( value );
    }
    else if constexpr ( std::is_same<T, std::string>::value )
    {
        m_writer->String( value );
    }
    else if constexpr ( std::is_same<T, float>::value )
    {
        m_writer->Double( value );
    }
    else
    {
        throw std::runtime_error( "Man you forgot to handle a type!!!" );
    }

    return *this;
}

template <typename T>
auto utilities::JsonSerializer::addKeyValuePair( const std::string& key, const T& value ) -> JsonSerializer&
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
        throw std::runtime_error( "Man you forgot to handle a type!!!" );
    }

    return *this;
}
