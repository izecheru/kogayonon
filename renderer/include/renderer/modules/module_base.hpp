#pragma once

struct VkExtent2D;

namespace rendering
{
class BaseModule
{
public:
  virtual ~BaseModule() = default;

  virtual auto registerPasses() -> void = 0;

protected:
  virtual auto createModuleResources( VkExtent2D extent ) -> void = 0;
  virtual auto destroyModuleResources() -> void = 0;

private:
};
} // namespace rendering