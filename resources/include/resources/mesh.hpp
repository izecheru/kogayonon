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
    uint32_t vertexOffset{ 0u };
    uint32_t indexOffset{ 0u };
    uint32_t indexCount{ 0u };
    Material submeshMaterial{};
    int materialIndex{ -1 };
};

class Mesh
{
  public:
    Mesh() = default;
    ~Mesh() = default;

    auto getVertices() -> std::vector<Vertex>&;
    auto getIndices() -> std::vector<uint32_t>&;

    auto getVertexBufferObject() -> graphics::VulkanBuffer&;
    auto getVerticesAllocation() -> VmaAllocation&;

    auto getIndicesBufferObject() -> graphics::VulkanBuffer&;
    auto getIndicesAllocation() -> VmaAllocation&;

    auto getSubmeshes() -> std::vector<Submesh>&;

    inline auto getPath() -> std::string&
    {
        return m_path;
    }

    inline auto setPath( const std::string& path ) -> void
    {
        m_path = path;
    }

    inline auto setLoaded( bool value ) -> void
    {
        m_loaded = value;
    }

    inline auto isLoaded() const -> bool
    {
        return m_loaded;
    }

  private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    bool m_loaded{ false };

    graphics::VulkanBuffer m_verticesBuff;
    graphics::VulkanBuffer m_indicesBuff;

    std::string m_path;
    std::vector<Submesh> m_submeshes;
};
} // namespace resources