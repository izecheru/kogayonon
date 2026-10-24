#pragma once
#include <entt/entt.hpp>
#include "core/ecs/entity.hpp"
#include "resources/model.hpp"
#include "core/ecs/registry.hpp"
#include "core/systems/player_input_system.hpp"

namespace core
{
class MouseMovedEvent;
} // namespace core

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

    auto deserialize( const std::filesystem::path& p ) -> void;
    auto serialize() -> void;

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
     * @param pModel The mesh weak_ptr from the asset manager
     */
    auto addMeshToEntity( entt::entity entity, resources::Model* pModel ) -> void;

    auto onUpdate() -> void;

    /**
     * @brief Removes the ModelComponent from the entity and clears the related data in the instance data struct
     * @param entity The entity we edit
     * @param pModel The model weak_ptr from asset manager
     */
    void removeMeshFromEntity( entt::entity entity );

    auto startPlaying() -> void;
    auto stopPlaying() -> void;
    auto getPlaying() const -> bool;

    auto getEntityCount() const -> uint32_t
    {
        return m_entityCount;
    }

    auto setRegistryModified( bool value ) -> void
    {
        m_registryModified = value;
    }

  private:
    bool m_playing;
    bool m_enabledEditorCamera;
    // this bool should be used to prepare entities for rendering
    bool m_registryModified{ false };
    glm::vec2 m_mouseCoord;

    std::mutex m_registryMutex;
    uint32_t m_entityCount;
    std::string m_name;
    std::unique_ptr<Registry> m_pRegistry;

    std::unique_ptr<PlayerInputSystem> m_inputSystem;
};
} // namespace core