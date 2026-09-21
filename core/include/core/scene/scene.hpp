#pragma once
#include "precompiled/pch.hpp"
#include <entt/entt.hpp>
#include "core/ecs/entity.hpp"
#include "resources/mesh.hpp"
#include "core/ecs/registry.hpp"

namespace core
{
class Scene
{
  public:
    explicit Scene( const std::string& name );
    ~Scene() = default;

    /**
     * @brief This returns the wrapper around entt::registry
     * @return Reference to Registry
     */
    auto getRegistry() -> Registry*;

    /**
     * @brief Used to get the entt::registry
     * @return Reference to entt::registry
     */
    auto getEnttRegistry() -> entt::registry&;

    auto getName() const -> std::string;
    auto changeName( const std::string& name ) -> void;

    inline auto getRegistryMutex() -> std::mutex&
    {
        return m_registryMutex;
    }

    /**
     * @brief Completely removes an entity from the registry
     * @param ent Entity id
     */
    auto removeEntity( entt::entity ent ) -> void;

    /**
     * @brief Creates a default entity with no components and adds it to the registry
     * @return Returns the freshly created entity
     */
    auto addEntity() -> entt::entity;

    /**
     * @brief Adds a model to an already existing entity in the scene registry
     * @param entity The entity id
     * @param pMesh The mesh weak_ptr from the asset manager
     */
    auto addMeshToEntity( entt::entity entity, resources::Mesh* pMesh ) -> void;

    auto onUpdate() -> void;

    /**
     * @brief Removes the MeshComponent from the entity and clears the related data in the instance data struct
     * @param entity The entity we edit
     * @param pModel The model weak_ptr from asset manager
     */
    void removeMeshFromEntity( entt::entity entity );

    inline auto getEntityCount() const -> uint32_t
    {
        return m_entityCount;
    }

    inline void setRegistryModified( bool value )
    {
        m_registryModified = value;
    }

  private:
    // this bool should be used to prepare entities for rendering
    bool m_registryModified{ false };

    std::mutex m_registryMutex;
    uint32_t m_entityCount;
    std::string m_name;
    std::unique_ptr<Registry> m_pRegistry;
};
} // namespace core