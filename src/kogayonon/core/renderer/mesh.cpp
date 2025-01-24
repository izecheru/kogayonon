#include <core/renderer/mesh.h>

void Mesh::draw() {
  m_vao.bind();
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_ebo.getIndices().size()), GL_UNSIGNED_INT, 0);
  m_vao.unbind();
}
