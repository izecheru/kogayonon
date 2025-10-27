#pragma once
#include <memory>
#include <string>
#include <vector>
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_resources
{
struct Submesh
{
  uint32_t vertexOffest{ 0 };
  uint32_t indexOffset{ 0 };
  uint32_t indexCount{ 0 };
};

class Mesh
{
public:
  Mesh() = default;
  ~Mesh() = default;

  Mesh( const Mesh& other );            // copy
  Mesh& operator=( const Mesh& other ); // copy assignment

  Mesh( Mesh&& other ) noexcept = default;            // move constructor
  Mesh& operator=( Mesh&& other ) noexcept = default; // move assignment

  explicit Mesh( const std::string& path, const std::vector<Vertex>&& vertices, const std::vector<uint32_t>&& indices,
                 const std::vector<Texture*>&& textures, const std::vector<Submesh>&& submeshes );

  explicit Mesh( const std::string& path, const std::vector<Vertex>&& vertices, const std::vector<uint32_t>&& indices,
                 const std::vector<Submesh>&& submeshes );

  std::vector<Vertex>& getVertices();
  std::vector<uint32_t>& getIndices();
  std::vector<Texture*>& getTextures();
  std::vector<Submesh>& getSubmeshes();

  inline std::string& getPath()
  {
    return m_path;
  }

  uint32_t& getVao();
  uint32_t& getVbo();
  uint32_t& getEbo();

private:
  std::vector<Texture*> m_textures;
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
  std::vector<Submesh> m_submeshes;

  uint32_t m_vao;
  uint32_t m_vbo;
  uint32_t m_ebo;

  std::string m_path;
};
} // namespace kogayonon_resources