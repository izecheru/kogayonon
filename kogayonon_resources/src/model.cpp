#include "resources/model.hpp"
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>

namespace kogayonon_resources
{
Model::Model( std::vector<Mesh>&& meshes, const std::string& path )
    : m_meshes{ std::move( meshes ) }
    , m_path{ path }
    , m_instanceMatrices{ glm::mat4{ 1.0f } }
    , m_instanceBuffer{ 0 }
    , m_amount{ 1 }
{
}

std::string Model::getPath()
{
  return m_path;
}

void Model::addInstance( const glm::mat4 instanceM )
{
  m_instanceMatrices.push_back( instanceM );
}

std::vector<glm::mat4>& Model::getInstances()
{
  return m_instanceMatrices;
}

unsigned int& Model::getInstanceBuffer()
{
  return m_instanceBuffer;
}

int Model::getAmount() const
{
  return m_amount;
}

void Model::setAmount( int amount )
{
  if ( amount > m_amount )
  {
    m_instanceMatrices.resize( amount, glm::mat4{ 1.0f } );

    float scale = 200.0f;
    for ( int i = 0; i < amount; i++ )
    {
      float x = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;
      float y = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;
      float z = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;

      m_instanceMatrices.at( i ) = glm::translate( glm::mat4( 1.0f ), glm::vec3( x, y, z ) );
    }
  }

  if ( m_instanceBuffer == 0 )
  {
    glCreateBuffers( 1, &m_instanceBuffer );

    // did not use glNamedBufferStorage cause it is immutable and instances change based on the amount of them
    glNamedBufferData( m_instanceBuffer, sizeof( glm::mat4 ) * m_instanceMatrices.size(), m_instanceMatrices.data(),
                       GL_DYNAMIC_DRAW );

    for ( int i = 0; i < m_meshes.size(); i++ )
    {
      const auto& vao = m_meshes.at( i ).getVao();

      glVertexArrayVertexBuffer( vao, 1, m_instanceBuffer, 0, sizeof( glm::mat4 ) );

      glEnableVertexArrayAttrib( vao, 3 );
      glEnableVertexArrayAttrib( vao, 4 );
      glEnableVertexArrayAttrib( vao, 5 );
      glEnableVertexArrayAttrib( vao, 6 );

      glVertexArrayAttribFormat( vao, 3, 4, GL_FLOAT, GL_FALSE, 0 );
      glVertexArrayAttribFormat( vao, 4, 4, GL_FLOAT, GL_FALSE, sizeof( glm::vec4 ) );
      glVertexArrayAttribFormat( vao, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof( glm::vec4 ) );
      glVertexArrayAttribFormat( vao, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof( glm::vec4 ) );

      glVertexArrayAttribBinding( vao, 3, 1 );
      glVertexArrayAttribBinding( vao, 4, 1 );
      glVertexArrayAttribBinding( vao, 5, 1 );
      glVertexArrayAttribBinding( vao, 6, 1 );

      glVertexArrayBindingDivisor( vao, 1, 1 );
    }
  }
  else
  {
    // Resize existing buffer (optional, only if m_amount changed)
    glNamedBufferData( m_instanceBuffer, sizeof( glm::mat4 ) * amount, nullptr, GL_DYNAMIC_DRAW );

    // Upload new data
    glNamedBufferSubData( m_instanceBuffer, 0, sizeof( glm::mat4 ) * amount, m_instanceMatrices.data() );
  }
  m_amount = amount;
}

std::vector<Mesh>& Model::getMeshes()
{
  return m_meshes;
}
} // namespace kogayonon_resources