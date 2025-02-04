#include <core/renderer/mesh.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "core/renderer/camera.h"
#include "core/logger.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) :mesh_buffers(vertices, indices), m_vertices(vertices), m_indices(indices) {
  setupMesh();
}

void Mesh::setupMesh() {
  mesh_buffers.vao->bind();
  mesh_buffers.vbo->bind();
  mesh_buffers.ebo->bind();

  mesh_buffers.vao->attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // Vertex positions
  //mesh_buffers.vao->attribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)); // Vertex normal
  //mesh_buffers.vao->attribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture)); // Vertex textures 

  mesh_buffers.vbo->unbind();
  mesh_buffers.ebo->unbind();
  mesh_buffers.vao->unbind();
}

void Mesh::draw() {
  mesh_buffers.bindBuffers();
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh_buffers.ebo->getCount()), GL_UNSIGNED_INT, 0);
  mesh_buffers.unbindBuffers();
}
