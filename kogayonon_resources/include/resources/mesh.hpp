#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "resources/skin.hpp"
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

  Mesh( const Mesh& other );
  Mesh& operator=( const Mesh& other );

  Mesh( Mesh&& other ) noexcept = default;
  Mesh& operator=( Mesh&& other ) noexcept = default;

  explicit Mesh( const std::string& path,
                 const std::vector<Vertex>&& vertices,
                 const std::vector<uint32_t>&& indices,
                 const std::vector<Texture*>&& textures,
                 const std::vector<Submesh>&& submeshes );

  explicit Mesh( const std::string& path,
                 const std::vector<Vertex>&& vertices,
                 const std::vector<uint32_t>&& indices,
                 const std::vector<Texture*>&& textures,
                 const std::vector<Submesh>&& submeshes,
                 std::optional<kogayonon_resources::Skeleton> skeleton );

  explicit Mesh( const std::string& path,
                 const std::vector<Vertex>&& vertices,
                 const std::vector<uint32_t>&& indices,
                 const std::vector<Submesh>&& submeshes );

  auto getVertices() -> std::vector<Vertex>&;
  auto getIndices() -> std::vector<uint32_t>&;
  auto getTextures() -> std::vector<Texture*>&;
  auto getSubmeshes() -> std::vector<Submesh>&;

  auto getPath() -> std::string&
  {
    return m_path;
  }

  auto getBones() -> std::optional<std::vector<glm::mat4>>
  {
    if ( !m_hasSkeleton )
      return {};

    std::vector<glm::mat4> bonesMatrices;
    for ( auto& bone : m_skeleton.joints )
    {
      bonesMatrices.emplace_back( bone.offsetMatrix );
    }
    return bonesMatrices;
  }

  auto getVao() -> uint32_t&;
  auto getVbo() -> uint32_t&;
  auto getEbo() -> uint32_t&;

private:
  std::vector<Texture*> m_textures;
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
  std::vector<Submesh> m_submeshes;

  kogayonon_resources::Skeleton m_skeleton;
  bool m_hasSkeleton{ false };

  uint32_t m_vao;
  uint32_t m_vbo;
  uint32_t m_ebo;

  std::string m_path;
};
} // namespace kogayonon_resources