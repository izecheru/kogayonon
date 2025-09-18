#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <string>

namespace kogayonon_core
{
class Registry;
} // namespace kogayonon_core

namespace kogayonon_core
{
class Scene
{
  public:
    Scene(const std::string& name);
    ~Scene() = default;

    Registry& getRegistry();
    entt::registry& getEnttRegistry();
    std::string getName() const;
    void changeName(const std::string& name);

  private:
    std::string m_name;
    std::unique_ptr<Registry> m_pRegistry;
};
} // namespace kogayonon_core