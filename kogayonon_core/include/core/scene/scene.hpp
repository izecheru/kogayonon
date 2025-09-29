#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/model_component.hpp"
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
  uint32_t instanceBuffer;
  // one per model
  std::vector<glm::mat4> instanceMatrices;
  // one per model
  int count = 1;
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
  void addDefaultEntity();
  void addModelEntity();

private:
  std::string m_name;
  std::unique_ptr<Registry> m_pRegistry;
  std::unordered_map<std::string, std::unique_ptr<InstanceData>> m_instances;
};
} // namespace kogayonon_core