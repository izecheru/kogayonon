#include <core/renderer/mesh.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "core/renderer/camera.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures) {
  this->vertices = vertices;
  this->indices = indices;
  this->textures = textures;

  m_vao.bind();

  m_vao.attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // Vertex positions
  m_vao.attribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // Vertex colors 
  m_vao.attribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)); // Vertex normal
  m_vao.attribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture)); // Vertex textures 

  m_vao.unbind();
}
