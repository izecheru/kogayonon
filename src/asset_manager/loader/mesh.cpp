#include "asset_manager/loader/mesh.h"

#include <glad/glad.h>

#include "registry_manager/registry_manager.h"
#include "klogger/klogger.h"
#include "renderer/camera.h"

namespace kogayonon
{
  //// Setup textures on main thread since opengl functions are not thread safe
  bool Mesh::setupTextures()
  {
    // auto texture_map = ContextManager::asset_manager()->getTextureMap();

    // if (m_textures.empty() || texture_map.empty())
    //   return false;

    // for (auto it = m_textures.begin(); it != m_textures.end(); ++it)
    //{
    //   Texture& texture = texture_map.at(*it);
    //   KLogger::log(LogType::INFO, "texture path:", texture.path);
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
    glCreateVertexArrays(1, &m_gpu->vao);

    // Upload vertex data directly to VBO
    glCreateBuffers(1, &m_gpu->vbo);
    glNamedBufferData(m_gpu->vbo, m_data->m_vertices.size() * sizeof(Vertex), m_data->m_vertices.data(), GL_STATIC_DRAW);

    // Upload vertex data directly to EBO
    glCreateBuffers(1, &m_gpu->ebo);
    glNamedBufferData(m_gpu->ebo, m_data->m_indices.size() * sizeof(uint32_t), m_data->m_indices.data(), GL_STATIC_DRAW);

    // Now we link the VBO to the VAO
    glVertexArrayVertexBuffer(m_gpu->vao, 0, m_gpu->vbo, 0, sizeof(Vertex));

    // Associate the EBO to the VAO
    glVertexArrayElementBuffer(m_gpu->vao, m_gpu->ebo);

    // We define the attributes directly on VAO
    glEnableVertexArrayAttrib(m_gpu->vao, 0);
    glEnableVertexArrayAttrib(m_gpu->vao, 1);
    glEnableVertexArrayAttrib(m_gpu->vao, 2);

    glVertexArrayAttribFormat(m_gpu->vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribFormat(m_gpu->vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
    glVertexArrayAttribFormat(m_gpu->vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coords));

    glVertexArrayAttribBinding(m_gpu->vao, 0, 0);
    glVertexArrayAttribBinding(m_gpu->vao, 1, 0);
    glVertexArrayAttribBinding(m_gpu->vao, 2, 0);

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
  //  glBindFramebuffer(m_fbo);
  //  glBindVertexArray(m_vao);
  //  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
  //  glBindVertexArray(0);
  //  glBindFramebuffer(0);
  //  glBindTextureUnit(0, 0);
  //}

  std::vector<Vertex>& Mesh::getVertices()
  {
    return m_data->m_vertices;
  }

  std::vector<uint32_t>& Mesh::getIndices()
  {
    return m_data->m_indices;
  }

  std::vector<std::string>& Mesh::getTextures()
  {
    return m_data->m_textures;
  }
} // namespace kogayonon