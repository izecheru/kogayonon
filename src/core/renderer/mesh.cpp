#include <core/renderer/mesh.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/logger.h"
#include "core/renderer/camera.h"

namespace kogayonon
{
  Mesh::Mesh(const std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures) : m_vertices(vertices), m_indices(indices), m_textures(textures) {
    Logger::logInfo("Mesh vertices ", vertices.size(), ", indices ", indices.size(), ", textures ", textures.size());
    this->setupMesh();
  }

  void Mesh::setupMesh() {
    // Now we are using Direct State Access
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glCreateBuffers(1, &m_ebo);

    // Upload vertex data directly to VBO
    glNamedBufferData(m_vbo, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    // Upload vertex data directly to EBO
    glNamedBufferData(m_ebo, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

    // Now we link the VBO to the VAO
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));
    // Associate the EBO to the VAO
    glVertexArrayElementBuffer(m_vao, m_ebo);

    // We define the attributes directly on VAO
    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribBinding(m_vao, 0, 0);

    glEnableVertexArrayAttrib(m_vao, 1);
    glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
    glVertexArrayAttribBinding(m_vao, 1, 0);

    glEnableVertexArrayAttrib(m_vao, 2);
    glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texture));
    glVertexArrayAttribBinding(m_vao, 2, 0);
  }

  void Mesh::draw(Shader& shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < m_textures.size(); i++)
    {
      std::string number;
      std::string name = m_textures[i].type;

      if (name == "texture_diffuse")
        number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
        number = std::to_string(specularNr++); // transfer unsigned int to string
      else if (name == "texture_normal")
        number = std::to_string(normalNr++); // transfer unsigned int to string
      else if (name == "texture_height")
        number = std::to_string(heightNr++); // transfer unsigned int to string

      std::string uniformName = name + number;
      int location = glGetUniformLocation(shader.getShaderId(), uniformName.c_str());
      if (location == -1)
      {
        Logger::logError("Uniform not found: ", uniformName);
      }
      else
      {
        glUniform1i(location, i);
      }

      glBindTextureUnit(i, m_textures[i].id);
    }

    // draw mesh
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
  }
}
