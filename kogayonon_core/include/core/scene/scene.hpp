#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <string>
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/entity.hpp"

namespace kogayonon_core
{
class Registry;
} // namespace kogayonon_core

namespace kogayonon_core
{
class Scene
{
public:
  Scene( const std::string& name );
  ~Scene() = default;

  Registry& getRegistry();
  entt::registry& getEnttRegistry();
  std::string getName() const;
  void changeName( const std::string& name );

private:
  std::string m_name;
  std::unique_ptr<Registry> m_pRegistry;

  // this is the instance map that we use for counting how many instances of the same model exist in the current scene
  std::unordered_map<std::string, uint32_t> m_instanceMap;
};
} // namespace kogayonon_core