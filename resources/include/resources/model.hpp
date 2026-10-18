#pragma once
#include <vulkan/vulkan.h>
#include "graphics/vulkan_buffer.hpp"
#include "resources/material.hpp"
#include "resources/skeleton.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace resources
{

/**
 * @brief Bounding box for entity culling on GPU
 * WIP
 */
struct AABB
{
    glm::vec3 min{};
    glm::vec3 max{};
};

/**
 * @brief Submesh struct
 *
 */
struct Mesh
{
    uint32_t vertexOffset{ 0u };
    uint32_t indexOffset{ 0u };
    uint32_t indexCount{ 0u };
    Material material{};
    int materialIndex{ -1 };
};

class Model
{
  public:
    Model() = default;
    ~Model() = default;

    auto getVertices() -> std::vector<Vertex>&;
    auto getIndices() -> std::vector<uint32_t>&;

    auto getMeshes() -> std::vector<Mesh>&;

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

    std::string m_path;
    std::vector<Mesh> m_meshes;
};
} // namespace resources