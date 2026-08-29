#pragma once

struct VkExtent2D;

namespace rendering
{
class BaseModule
{
public:
  virtual ~BaseModule() = default;

  virtual auto registerPasses() -> void = 0;
  virtual auto setExtent( VkExtent2D extent ) -> void = 0;

  virtual auto recreate( VkExtent2D extent ) -> void
  {
    throw std::runtime_error( "this is not yet implemented" );
  }

protected:
  virtual auto createModuleResources( VkExtent2D extent ) -> void = 0;
  virtual auto destroyModuleResources() -> void = 0;
};
} // namespace rendering