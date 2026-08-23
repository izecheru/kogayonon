#include "core/asset_manager/tinygltf_loader.hpp"
#include "utilities/utils/utils.hpp"

auto core::TinyGltfLoader::loadModel( std::string_view path ) -> void
{
  tg3_error_code err = tg3_parse_file( &m_model, &m_errors, std::string{ path }.c_str(), 10, &m_opts );

  if ( err != TG3_OK )
  {
    for ( auto i = 0u; i < m_errors.count; i++ )
    {
      const char* message = m_errors.entries[i].message;
      K_ERROR( "[{}] {}", i, message ? message : "null" );
    }
  }

  tg3_model_free( &m_model );
  tg3_error_stack_free( &m_errors );
}