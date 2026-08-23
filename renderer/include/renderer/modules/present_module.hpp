#pragma once
#include "graphics/vulkan_image.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "renderer/frame_graph.hpp"

namespace rendering
{
struct PresentModuleData
{
  FGResource* finalTexture;
};

class PresentModule
{
public:
  PresentModule();
  ~PresentModule();

private:
};
} // namespace rendering