#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

struct Vertex
{
  glm::vec3 positions;
  glm::vec3 colors;
  glm::vec2 texture;
};

class VertexBufferObject
{
public:
  VertexBufferObject(const std::vector<Vertex>& vertices);

  VertexBufferObject() = default;
  ~VertexBufferObject() { if (m_id != 0)glDeleteBuffers(1, &m_id); }

  void bind();
  void unbind();

private:
  unsigned int m_id;
};

