#pragma once
#include "core/ecs/entity.hpp"
#include "precompiled/pch.hpp"
#include "resources/mesh.hpp"
#include <entt/entt.hpp>

namespace core
{
class Registry;
struct DirectionalLightComponent;
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
  void changeName( const std::string& name );

  inline auto getRegistryMutex() -> std::mutex&
  {
    return m_registryMutex;
  }

  /**
   * @brief Completely removes an entity from the registry
   * @param ent Entity id
   */
  void removeEntity( entt::entity ent );

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
  void addMeshToEntity( entt::entity entity, resources::Mesh* pMesh );

  /**
   * @brief Removes the MeshComponent from the entity and clears the related data in the instance data struct
   * @param entity The entity we edit
   * @param pModel The model weak_ptr from asset manager
   */
  void removeMeshFromEntity( entt::entity entity );

  /**
   * @brief Iterates through the entities that have rigid bodies and
   * take the transforms from there and apply them to the models
   */
  void updateRigidbodyEntities();

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