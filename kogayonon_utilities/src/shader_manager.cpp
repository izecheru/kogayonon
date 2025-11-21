#include "utilities/shader_manager/shader_manager.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace kogayonon_utilities
{
void ShaderManager::markForRecompilation( const std::string& filePath )
{
  for ( auto& shaders : m_shaders )
  {
    auto fragment = shaders.second.getFragmentShaderPath();
    auto vertex = shaders.second.getVertexShaderPath();
    std::replace( fragment.begin(), fragment.end(), '/', '\\' );
    std::replace( vertex.begin(), vertex.end(), '/', '\\' );
    if ( fragment == filePath || vertex == filePath )
    {
      shaders.second.markForCompilation();
    }
  }
}

void ShaderManager::pushShader( const std::string& vertex_shader, const std::string& fragment_shader,
                                const std::string& shader_name )
{
  if ( m_shaders.contains( shader_name ) )
  {
    m_shaders.at( shader_name ).destroy();
    m_shaders.erase( shader_name );
  }

  Shader sh;
  sh.initializeShaderSource( vertex_shader, fragment_shader );
  m_shaders.emplace( shader_name, std::move( sh ) );
}

void ShaderManager::compileMarkedShaders()
{
  for ( auto& shader : m_shaders )
  {
    if ( shader.second.isCompiled() == false )
      shader.second.initializeProgram();
  }
}

Shader& ShaderManager::getShader( const std::string& shader_name )
{
  if ( m_shaders.find( shader_name ) == m_shaders.end() )
  {
    throw std::runtime_error( "Shader not found" );
  }
  return m_shaders.at( shader_name );
}

unsigned int ShaderManager::getShaderId( const std::string& shader_name )
{
  return getShader( shader_name ).getShaderId();
}

void ShaderManager::bindShader( const std::string& shader_name )
{
  getShader( shader_name ).bind();
}

void ShaderManager::unbindShader( const std::string& shader_name )
{
  getShader( shader_name ).unbind();
}

void ShaderManager::removeShader( const std::string& shaderName )
{
  if ( const auto& it = m_shaders.find( shaderName ); it != m_shaders.end() )
  {
    m_shaders.erase( it );
  }
}
} // namespace kogayonon_utilities