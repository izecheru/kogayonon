#pragma once
#include <vector>
#include <glm/ext/vector_float3.hpp>

struct Vertex
{
  glm::vec3 positions;
  glm::vec3 colors;
};

class VertexBufferObject
{
public:
  VertexBufferObject(const std::vector<Vertex>& vertices);

  void bind();
  void unbind();

private:
  unsigned int m_id;
};

class ElementsBufferObject
{
public:
  ElementsBufferObject(const std::vector<unsigned int>& indices);

  void bind();
  void unbind();

  const std::vector<unsigned int> getIndices() const;

private:
  unsigned int m_id;
  std::vector<unsigned int> indices;
};

class VertexArrayObject
{
public:
  VertexArrayObject();

  void bind();
  void unbind();

  void attribPointer(unsigned int index, int size, unsigned int type, bool normalized, unsigned int stride, const void* offset);
private:
  unsigned int m_id;
};