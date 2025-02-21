#include <core/model_loader/mesh.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/model_loader/model_loader.h"
#include "core/logger.h"
#include "core/renderer/camera.h"
#include <stb\stb_image.h>
#define CHECK_GL_ERROR() \
    { GLenum err; while ((err = glGetError()) != GL_NO_ERROR) { Logger::logError("OpenGL Error: ", err); }}
namespace kogayonon
{
  Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t >& indices, std::vector<Texture>& textures) :
    m_vertices(vertices), m_indices(indices), m_textures(textures),
    m_ebo(0), m_vao(0), m_vbo(0)
  {
    //unsigned int max_index = *std::max_element(m_indices.begin(), m_indices.end());
    //Logger::logError("Max index value: ", max_index);
    Logger::logError("v[", vertices.size(), "]", " i[", indices.size(), "] ", "t[", textures.size(), "]");
  }

  // Setup textures on main thread since opengl functions are not thread safe
  void Mesh::setupTextures()
  {
    for (Texture& texture : m_textures)
    {
      if (texture.data != nullptr)
      {

        assert((1 <= texture.num_components) && (4 >= texture.num_components));
        GLenum glformat = GL_RGB;
        switch (texture.num_components)
        {
          case 1: glformat = GL_RED;  break;
          case 2: glformat = GL_RG;   break;
          case 3: glformat = GL_RGB;  break;
          case 4: glformat = GL_RGBA; break;
        }
        glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
        // We allocate immutable storage for the texture
        //int levels = static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;
        glTextureStorage2D(texture.id, 1, GL_RGBA8, texture.width, texture.height);

        // Upload the image data to the texture
        glTextureSubImage2D(texture.id, 0, 0, 0, texture.width, texture.height, glformat, GL_UNSIGNED_BYTE, texture.data);

        // Generate mipmaps
        glGenerateTextureMipmap(texture.id);

        glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_REPEAT); // Or GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, etc.
        glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_REPEAT); // For the T coordinate
        glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        assert(texture.data != nullptr);
        stbi_image_free(texture.data);
        texture.data = nullptr;
      }
      else
      {
        Logger::logError("Failed to load image from ", texture.path);
      }
    }
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

    GLint vbo_size = 0, ebo_size = 0;
    glGetNamedBufferParameteriv(m_vbo, GL_BUFFER_SIZE, &vbo_size);
    glGetNamedBufferParameteriv(m_ebo, GL_BUFFER_SIZE, &ebo_size);

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
    glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texture));

    glVertexArrayAttribBinding(m_vao, 0, 0);
    glVertexArrayAttribBinding(m_vao, 1, 0);
    glVertexArrayAttribBinding(m_vao, 2, 0);

    // setup textures here
    setupTextures();
    m_num_indices = m_indices.size();
    m_init = true;
  }

  void Mesh::draw(Shader& shader)
  {
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
        Logger::logInfo("Uniform set:", uniformName);
        glUniform1i(location, i);
      }

      glBindTextureUnit(i, m_textures[i].id);
    }

    // draw mesh
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<uint32_t>(m_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
  }
}
