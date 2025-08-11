#include "core/asset_manager/loader/mesh.h"

#include <glad/glad.h>

#include "core/klogger/klogger.h"
#include "core/renderer/camera.h"

namespace kogayonon
{
  Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<std::string>& textures)
      : m_vertices(vertices), m_indices(indices), m_textures(textures)
  {}

  Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
      : m_vertices(vertices), m_indices(indices), m_textures()
  {}

  //// Setup textures on main thread since opengl functions are not thread safe
  bool Mesh::setupTextures()
  {
    // //if (textures_map.empty())
    //   return false;

    // for (unsigned int i = 0; i < m_textures.size(); i++)
    //{
    //   auto& texture = textures_map[m_textures[i]];
    //   KLogger::log(LogType::INFO, "texture path:", m_textures[i]);
    //   if (!texture.data.empty())
    //   {
    //     GLenum glformat = GL_RGB;
    //     switch (texture.num_components)
    //     {
    //     case 1:
    //       glformat = GL_RED;
    //       break;
    //     case 2:
    //       glformat = GL_RG;
    //       break;
    //     case 3:
    //       glformat = GL_RGB;
    //       break;
    //     case 4:
    //       glformat = GL_RGBA;
    //       break;
    //     }
    //     glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);

    //    // We allocate immutable storage for the texture
    //    glTextureStorage2D(texture.id, 1, GL_RGBA8, texture.width, texture.height);

    //    // Upload the image data to the texture
    //    glTextureSubImage2D(texture.id, 0, 0, 0, texture.width, texture.height, glformat, GL_UNSIGNED_BYTE, texture.data.data());

    //    // Generate mipmaps
    //    glGenerateTextureMipmap(texture.id);

    //    glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_REPEAT); // Or GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, etc.
    //    glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_REPEAT); // For the T coordinate
    //    glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //    glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //    texture.data.clear();
    //  }
    //  else
    //  {
    //    KLogger::log(LogType::ERROR, "Failed to load image from ", texture.path);
    //  }
    //}
    return true;
  }

  void Mesh::setupMesh()
  {
    glCreateVertexArrays(1, &m_vao);

    // Upload vertex data directly to VBO
    glCreateBuffers(1, &m_vbo);
    glNamedBufferData(m_vbo, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    // Upload vertex data directly to EBO
    glCreateBuffers(1, &m_ebo);
    glNamedBufferData(m_ebo, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

    assert(!m_vertices.empty() && !m_indices.empty());

    // Now we link the VBO to the VAO
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));

    // Associate the EBO to the VAO
    glVertexArrayElementBuffer(m_vao, m_ebo);

    assert(m_vao != 0 && m_vbo != 0 && m_ebo != 0);

    // We define the attributes directly on VAO
    glEnableVertexArrayAttrib(m_vao, 0);
    glEnableVertexArrayAttrib(m_vao, 1);
    glEnableVertexArrayAttrib(m_vao, 2);

    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
    glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coords));

    glVertexArrayAttribBinding(m_vao, 0, 0);
    glVertexArrayAttribBinding(m_vao, 1, 0);
    glVertexArrayAttribBinding(m_vao, 2, 0);

    m_init = setupTextures();
  }

  // void Mesh::draw(const Shader& shader)
  //{
  //   if (!m_init)
  //     setupMesh();

  //  TextureManager* manager = TextureManager::getInstance();
  //  auto& textures_map      = manager->getTextures();
  //  for (unsigned int i = 0; i < m_textures.size(); i++)
  //  {
  //    glBindTextureUnit(i, textures_map[m_textures[i]].id);
  //  }

  //  // draw mesh
  //  glBindVertexArray(m_vao);
  //  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
  //  glBindVertexArray(0);
  //  glBindTextureUnit(0, 0);
  //}

  Mesh::vertice_vec& Mesh::getVertices()
  {
    return m_vertices;
  }

  Mesh::indices_vec& Mesh::getIndices()
  {
    return m_indices;
  }

  Mesh::texture_vec& Mesh::getTextures()
  {
    return m_textures;
  }
} // namespace kogayonon