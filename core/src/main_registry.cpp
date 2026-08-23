#include "core/ecs/main_registry.hpp"

namespace core
{
MainRegistry::~MainRegistry()
{
  getContext<std::shared_ptr<AssetManager>>().reset();
}
} // namespace core