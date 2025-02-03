#pragma once
#include <glad/glad.h>
#include <vector>

class ElementsBufferObject
{
public:
  ElementsBufferObject() = default;

  ElementsBufferObject(ElementsBufferObject&& other)noexcept;
  ElementsBufferObject& operator=(ElementsBufferObject&& other)noexcept;

  ElementsBufferObject(const std::vector<unsigned int>& indices);
  ~ElementsBufferObject() { if (m_id != 0)glDeleteBuffers(1, &m_id); }

  void bind();
  void unbind();

  const std::vector<unsigned int> getIndices() const;

private:
  unsigned int m_id = 0;
  std::vector<unsigned int> indices;
};
