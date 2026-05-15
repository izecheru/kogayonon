#pragma once
#include <vulkan/vulkan.h>
#include "graphics/vulkan_buffer.hpp"
#include "precompiled/pch.hpp"
#include "resources/material.hpp"
#include "resources/skeleton.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace resources
{
struct Submesh
{
  uint32_t vertexOffset;
  uint32_t indexOffset;
  uint32_t indexCount;
  Material submeshMaterial;
  int materialIndex;
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
                 const std::vector<Texture*>&& textures );

  explicit Mesh( const std::string& path,
                 const std::vector<Vertex>&& vertices,
                 const std::vector<uint32_t>&& indices,
                 const std::vector<Texture*>&& textures,
                 std::optional<resources::Skeleton> skeleton );

  explicit Mesh( const std::string& path, const std::vector<Vertex>&& vertices, const std::vector<uint32_t>&& indices );

  auto getVertices() -> std::vector<Vertex>&;
  auto getIndices() -> std::vector<uint32_t>&;
  auto getTextures() -> std::vector<Texture*>&;

  auto getVertexBufferObject() -> graphics::VulkanBuffer&;
  auto getVerticesAllocation() -> VmaAllocation&;

  auto getIndicesBufferObject() -> graphics::VulkanBuffer&;
  auto getIndicesAllocation() -> VmaAllocation&;

  auto getSubmeshes() -> std::vector<Submesh>&;

  inline auto getPath() -> std::string&
  {
    return m_path;
  }

private:
  std::vector<Texture*> m_textures;
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;

  int m_materialIndex;

  graphics::VulkanBuffer m_verticesBuff;
  graphics::VulkanBuffer m_indicesBuff;

  std::string m_path;
  std::vector<Submesh> m_submeshes;
};
} // namespace resources