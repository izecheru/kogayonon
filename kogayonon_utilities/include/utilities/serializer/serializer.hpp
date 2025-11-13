#pragma once
#include <assert.h>
#include <fstream>

namespace kogayonon_utilities
{
class Serializer
{
public:
  /**
   * @brief Serialize a simple variable, like the size of a vector so we know how big is the data returned
   * @tparam T Type of variable
   * @param data Value of variable
   * @param out Ofstream
   * @return True if everything works fine
   */
  template <typename T>
  static bool serialize( const T& data, std::fstream& out )
  {
    assert( out.is_open() );
    out.write( reinterpret_cast<const char*>( &data ), sizeof( T ) );
    return out.good();
  }

  /**
   * @brief Deserealizes a value into a variable
   * @tparam T Type of variable
   * @param data
   * @param in Ifstream file
   * @return True if everything works fine
   */
  template <typename T>
  static bool deserialize( T& data, std::fstream& in )
  {
    assert( in.is_open() );
    in.read( reinterpret_cast<char*>( &data ), sizeof( T ) );
    return in.good();
  }

  static bool serialize( const void* data, size_t size, std::fstream& out )
  {
    assert( out.is_open() );
    out.write( reinterpret_cast<const char*>( data ), size );
    return out.good();
  }

  static bool deserialize( void* data, size_t size, std::fstream& in )
  {
    assert( in.is_open() );
    in.read( reinterpret_cast<char*>( data ), size );
    return in.good();
  }

private:
  Serializer() = delete;
  ~Serializer() = delete;
};
} // namespace kogayonon_utilities