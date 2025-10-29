#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include "core/ecs/entity.hpp"
#include "rendering/light_shader_storagebuffer.hpp"
#include "rendering/lightcount_uniformbuffer.hpp"
#include "resources/mesh.hpp"

namespace kogayonon_core
{
class Registry;
} // namespace kogayonon_core

namespace kogayonon_core
{
struct InstanceData
{
  // buffer for entity ids used for picking
  uint32_t entityIdBuffer{ 0 };

  // entity ids vector
  std::vector<int> entityIds{};

  // the buffer in which we upload the instance matrices
  uint32_t instanceBuffer{ 0 };

  // each instance has its own instance matrix that enables transformations
  std::vector<glm::mat4> instanceMatrices{ 0 };

  // the amount of instances that will be drawn for a specific model using glDrawElementsInstanced
  int count{ 1 };

  // pointer to the mesh, we use this as a key in unordered_map<Model*,unique_ptr<InstanceData>>
  kogayonon_resources::Mesh* pMesh{ nullptr };
};

class Scene
{
public:
  Scene( const std::string& name );
  ~Scene() = default;

  Registry& getRegistry();
  entt::registry& getEnttRegistry();
  std::string getName() const;
  void changeName( const std::string& name );

  inline std::mutex& getRegistryMutex()
  {
    return m_registryMutex;
  }

  void prepareForRendering();

  /**
   * @brief Removes instance data tied to the model component this entity has
   * @param ent The entity Id
   */
  void removeInstanceData( entt::entity ent );

  /**
   * @brief Completely removes an entity from the registry
   * @param ent Entity id
   */
  void removeEntity( entt::entity ent );

  /**
   * @brief Creates a default entity with no components and adds it to the registry
   * @return Returns the freshly created entity
   */
  Entity addEntity();

  /**
   * @brief Initializes the instance data for a particular model pointer
   * @param entityId Id of the entity we get the model component from
   * @return True if the instances vector has the model pointer, false otherwise
   */
  bool addInstanceData( entt::entity entityId );

  /**
   * @brief Adds a model to an already existing entity in the scene registry
   * @param entity The entity id
   * @param pMesh The mesh weak_ptr from the asset manager
   */
  void addMeshToEntity( entt::entity entity, kogayonon_resources::Mesh* pMesh );

  /**
   * @brief Removes the MeshComponent from the entity and clears the related data in the instance data struct
   * @param entity The entity we edit
   * @param pModel The model weak_ptr from asset manager
   */
  void removeMeshFromEntity( entt::entity entity );

  /**
   * @brief Get instance data for a specified model since Model* is a key used in the instance map
   * @param pModel The model weak_ptr
   * @return Returns an InstanceData* to the instance data linked to the model
   */
  InstanceData* getData( kogayonon_resources::Mesh* pModel );

  /**
   * @brief Sets up the instance buffer, entity buffer and uploads the required data for rendering to the gpu
   * @param data InstanceData* (pointer) to the data that we are about to upload to the gpu
   */
  void setupMultipleInstances( InstanceData* data );

  void addPointLight();
  void addPointLight( entt::entity entityId );

  void bindLightBuffers();
  void unbindLightBuffers();

  void updateLightBuffers();

  kogayonon_resources::PointLight& getPointLight( uint32_t index );

  inline uint32_t getEntityCount() const
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
  std::unordered_map<kogayonon_resources::Mesh*, std::unique_ptr<InstanceData>> m_instances;

  kogayonon_rendering::LightCountUniformbuffer m_lightUBO;
  kogayonon_rendering::LightShaderStoragebuffer m_lightSSBO;
};
} // namespace kogayonon_core