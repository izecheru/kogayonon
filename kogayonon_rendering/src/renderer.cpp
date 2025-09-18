#include "rendering/renderer.hpp"
#include <glad/glad.h>
#include "resources/vertex.hpp"

namespace kogayonon_rendering
{
Renderer::Renderer()
    : m_vao( 0 )
    , m_vbo( 0 )
    , m_ebo( 0 )
{
}

Renderer::~Renderer()
{
  if ( m_vao )
  {
    glDeleteVertexArrays( 1, &m_vao );
  }
  if ( m_ebo )
  {
    glDeleteBuffers( 1, &m_ebo );
  }
  if ( m_vbo )
  {
    glDeleteBuffers( 1, &m_vbo );
  }
}

void Renderer::initialise()
{
  glCreateBuffers( 1, &m_vbo );
  glNamedBufferStorage( m_vbo, sizeof( kogayonon_resources::Vertex ) * MAX_VERT_COUNT, nullptr,
                        GL_DYNAMIC_STORAGE_BIT );

  glCreateBuffers( 1, &m_ebo );
  glNamedBufferStorage( m_ebo, sizeof( kogayonon_resources::Vertex ) * MAX_IND_COUNT, nullptr, GL_DYNAMIC_STORAGE_BIT );

  glCreateVertexArrays( 1, &m_vao );
  glVertexArrayVertexBuffer( m_vao, 0, m_vbo, 0, sizeof( kogayonon_resources::Vertex ) );
  glVertexArrayElementBuffer( m_vao, m_ebo );

  glEnableVertexArrayAttrib( m_vao, 0 );
  glEnableVertexArrayAttrib( m_vao, 1 );
  glEnableVertexArrayAttrib( m_vao, 2 );

  glVertexArrayAttribFormat( m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof( kogayonon_resources::Vertex, position ) );
  glVertexArrayAttribFormat( m_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof( kogayonon_resources::Vertex, normal ) );
  glVertexArrayAttribFormat( m_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof( kogayonon_resources::Vertex, textureCoords ) );

  glVertexArrayAttribBinding( m_vao, 0, 0 );
  glVertexArrayAttribBinding( m_vao, 1, 0 );
  glVertexArrayAttribBinding( m_vao, 2, 0 );
}
} // namespace kogayonon_rendering