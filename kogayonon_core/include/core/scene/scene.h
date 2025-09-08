#pragma once
#include <memory>
#include <string>

namespace kogayonon_core
{
class Registry;
}

namespace kogayonon_core
{
class Scene
{
  public:
    Scene(const std::string& name);
    ~Scene() = default;

    Registry& getRegistry();

  private:
    std::string m_name;
    std::unique_ptr<Registry> m_pRegistry;
};
} // namespace kogayonon_core