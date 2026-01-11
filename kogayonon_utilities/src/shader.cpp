#include "utilities/shader_manager/shader.hpp"
#include <assert.h>
#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

namespace kogayonon_utilities
{

void Shader::initializeShaderSource( const std::string& vertexPath, const std::string& fragmentPath )
{
  m_shaderSource = parseShaderFile( vertexPath, fragmentPath );
}

void Shader::initializeProgram()
{
  // cache the paths
  std::string v = m_shaderSource.vertexPath.string();
  std::string f = m_shaderSource.fragmentPath.string();

  // reparse the files
  m_shaderSource = parseShaderFile( v, f );
  m_programId = createShader();

  // mark compiled
  m_isCompiled = true;
}

bool Shader::isCompiled() const
{
  return m_isCompiled;
}

std::string Shader::getVertexShaderPath()
{
  return m_shaderSource.vertexPath.string();
}

std::string Shader::getFragmentShaderPath()
{
  return m_shaderSource.fragmentPath.string();
}

shader_source Shader::parseShaderFile( const std::string& vertPath, const std::string& fragPath )
{
  std::ifstream vertexStream( vertPath );
  if ( !vertexStream.is_open() )
  {
    spdlog::error( "Failed to open shader file {}", vertPath );
    std::string result = "";
    assert( result.size() > 0 );
    return { result, result };
  }

  std::stringstream vertex_ss;
  std::string line;

  while ( getline( vertexStream, line ) )
  {
    vertex_ss << line << '\n';
  }

  std::ifstream fragmentStream( fragPath );
  if ( !fragmentStream.is_open() )
  {
    spdlog::error( "Failed to open shader file {} ", fragPath );
    std::string result = "";

    assert( result.size() > 0 );
    return { result, result };
  }

  std::stringstream fragment_ss; // 0 for vertex, 1 for fragment
  line = "";
  while ( getline( fragmentStream, line ) )
  {
    fragment_ss << line << '\n';
  }
  std::string vertex = vertex_ss.str();
  std::string fragment = fragment_ss.str();

  shader_source source{
    .vertexSource = vertex, .fragmentSource = fragment, .vertexPath = vertPath, .fragmentPath = fragPath };
  return source;
}

void Shader::bind() const
{
  glUseProgram( m_programId );
}

void Shader::unbind() const
{
  glUseProgram( 0 );
}

void Shader::setInt( const char* uniform, int value ) const
{
  if ( int location = glGetUniformLocation( m_programId, uniform ); location == -1 )
  {
    // Uniform not found, print a warning or error message
    spdlog::error( "Uniform not found {} ", uniform );
  }
  else
  {
    glUniform1i( location, value ); // Set the uniform value
  }
}

void Shader::setMat4( const char* uniform, const glm::mat4& mat )
{
  if ( int location = glGetUniformLocation( m_programId, uniform ); location == -1 )
  {
    spdlog::error( "Uniform not found {} ", uniform );
  }
  else
  {
    glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr( mat ) );
  }
}

void Shader::setBool( const char* uniform, bool value ) const
{
  if ( int location = glGetUniformLocation( m_programId, uniform ); location == -1 )
  {
    spdlog::error( "Uniform not found {} ", uniform );
  }
  else
  {
    glUniform1i( location, value );
  }
}

auto Shader::getShaderId() const -> uint32_t
{
  return m_programId;
}

auto Shader::compileShader( uint32_t shader_type, std::string& source_data ) -> uint32_t
{
  auto id = glCreateShader( shader_type );
  const char* m_shaderSource = source_data.c_str();
  glShaderSource( id, 1, &m_shaderSource, nullptr );
  glCompileShader( id );
  int result;
  glGetShaderiv( id, GL_COMPILE_STATUS, &result );
  if ( result == GL_FALSE )
  {
    int length;
    glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length );
    auto message = (char*)_malloca( length * sizeof( char ) );
    glGetShaderInfoLog( id, length, &length, message );
    if ( shader_type == GL_VERTEX_SHADER )
    {
      spdlog::info( "Failed to compile vertex shader:{}", message );
    }
    else if ( shader_type == GL_FRAGMENT_SHADER )
    {
      spdlog::info( "Failed to compile fragment shader:{}", message );
    }
    glDeleteShader( id );
    return 0;
  }
  spdlog::info( "Shader compilation is done" );
  ;
  return id;
}

void Shader::destroy() const
{
  glDeleteProgram( m_programId );
}

void Shader::markForCompilation()
{
  m_isCompiled = false;
}

auto Shader::createShader() -> uint32_t
{
  uint32_t program = glCreateProgram();
  uint32_t vs = compileShader( GL_VERTEX_SHADER, m_shaderSource.vertexSource );
  uint32_t fs = compileShader( GL_FRAGMENT_SHADER, m_shaderSource.fragmentSource );

  // Attach shaders now
  glAttachShader( program, vs );
  glAttachShader( program, fs );
  glLinkProgram( program );
  int result;
  glGetProgramiv( program, GL_LINK_STATUS, &result );
  if ( result == GL_FALSE )
  {
    int length;
    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &length );
    auto message = (char*)malloc( length * sizeof( char ) );
    glGetProgramInfoLog( program, length, &length, message );

    spdlog::info( "Failed to link shader program {}", message );
    free( message );
    glDeleteProgram( program );
    return 0;
  }
  glValidateProgram( program );
  glDeleteShader( vs );
  glDeleteShader( fs );

  spdlog::info( "Succesfully linked shaders" );
  return program;
}
} // namespace kogayonon_utilities