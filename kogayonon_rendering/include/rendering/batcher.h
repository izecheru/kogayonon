#pragma once
#include <glad/glad.h>
#include <memory>
#include <vector>

namespace kogayonon_resources
{
class Model;
class Texture;
} // namespace kogayonon_resources

namespace kogayonon_utilities
{
class Shader;
} // namespace kogayonon_utilities

namespace kogayonon_rendering
{

template <typename TBatch>
class Batcher
{
  Batcher();
  ~Batcher() = default;

  void init();

  unsigned int getVao();
  unsigned int getVbo();
  unsigned int getIbo();

  void setVertexAttribute( unsigned int layoutPos, unsigned int numComponents, GLenum type, GLsizeiptr stride,
                           void* offset );

  template <typename TElement>
  void addToBatch( TElement& element );

protected:
  std::vector<std::unique_ptr<TBatch>> m_batches;

private:
  unsigned int m_vao, m_vbo, m_ibo;
};

template <typename TBatch>
unsigned int Batcher<TBatch>::getVao()
{
  return m_vao;
}

template <typename TBatch>
unsigned int Batcher<TBatch>::getVbo()
{
  return m_vbo;
}

template <typename TBatch>
unsigned int Batcher<TBatch>::getIbo()
{
  return m_ibo;
}

template <typename TBatch>
template <typename TElement>
void Batcher<TBatch>::addToBatch( TElement& element )
{
}

template <typename TBatch>
void Batcher<TBatch>::setVertexAttribute( unsigned int layoutPos, unsigned int numComponents, GLenum type,
                                          GLsizeiptr stride, void* offset )
{
  glEnableVertexArrayAttrib( m_vao, layoutPos );
  glVertexArrayAttribFormat( m_vao, layoutPos, numComponents, type, GL_FALSE, (GLuint)offset );
  glVertexArrayAttribBinding( m_vao, layoutPos, 0 );
  glVertexArrayVertexBuffer( m_vao, 0, m_vbo, 0, stride );
}

template <typename TBatch>
void Batcher<TBatch>::init()
{
  glCreateVertexArrays( 1, &m_vao );
  glCreateBuffers( 1, &m_vbo );

  constexpr size_t maxVertices = 1000;
  constexpr size_t vertexSize = 4 * sizeof( float );
  glNamedBufferStorage( m_vbo, maxVertices * vertexSize, nullptr, GL_DYNAMIC_STORAGE_BIT );

  glVertexArrayVertexBuffer( m_vao, 0, m_vbo, 0, vertexSize );

  setVertexAttribute( 0, 2, GL_FLOAT, 4 * sizeof( float ), 0 );
  setVertexAttribute( 1, 2, GL_FLOAT, 4 * sizeof( float ), 2 * sizeof( float ) );

  glCreateBuffers( 1, &m_ibo );
  glNamedBufferStorage( m_ibo, maxVertices * sizeof( unsigned int ), nullptr, GL_DYNAMIC_STORAGE_BIT );
  glVertexArrayElementBuffer( m_vao, m_ibo );
}

} // namespace kogayonon_rendering