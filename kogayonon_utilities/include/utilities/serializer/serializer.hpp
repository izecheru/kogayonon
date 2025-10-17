#pragma once
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
  static bool serialize( T& data, std::fstream& out )
  {
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
    in.read( reinterpret_cast<char*>( &data ), sizeof( T ) );

    return in.good();
  }

  /**
   * @brief Serialize structures like vectors
   * @tparam V Type of structure
   * @param data Pointer to the data ( vector.data() )
   * @param size Size of the data (element count * sizeof(V))
   * @return true if everything worked ok
   */
  template <typename V>
  static bool serialize( const V* data, size_t size, std::fstream& out )
  {
    out.write( reinterpret_cast<const char*>( data ), size );
    return out.good();
  }

  /**
   * @brief Deserealizes structures like vectors
   * @tparam V Typ of structure
   * @param data Data pointer vector.data()
   * @param size The size of elements we are about to read elementCount * sizeof(varible type)
   * @param in Ifsteram
   * @return True if everything works fine
   */
  template <typename V>
  static bool deserialize( V* data, size_t size, std::fstream& in )
  {
    in.read( reinterpret_cast<char*>( data ), size );
    return in.good();
  }

private:
  Serializer() = delete;
  ~Serializer() = delete;
};
} // namespace kogayonon_utilities