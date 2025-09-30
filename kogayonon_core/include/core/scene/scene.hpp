#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include "core/ecs/entity.hpp"
#include "resources/model.hpp"

namespace kogayonon_core
{
class Registry;
} // namespace kogayonon_core

namespace kogayonon_core
{
struct InstanceData
{
  // one per model
  uint32_t instanceBuffer{ 0 };

  // one per model
  std::vector<glm::mat4> instanceMatrices{};

  // one per model
  int count = 1;

  kogayonon_resources::Model* pModel{ nullptr };
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

  void removeEntity( entt::entity ent );
  void addEntity();
  void addEntity( std::weak_ptr<kogayonon_resources::Model> pModel );
  InstanceData* getData( kogayonon_resources::Model* pModel );

private:
  void setupInstance( InstanceData* data );

private:
  std::string m_name;
  std::unique_ptr<Registry> m_pRegistry;
  std::unordered_map<kogayonon_resources::Model*, std::unique_ptr<InstanceData>> m_instances;
};
} // namespace kogayonon_core