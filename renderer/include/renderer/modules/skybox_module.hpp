#pragma once
#include "graphics/vulkan_pipeline.hpp"
#include "graphics/vulkan_descriptor.hpp"
#include "graphics/vulkan_context.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/module_base.hpp"
#include "renderer/modules/module_rendering_info.hpp"

namespace rendering
{

namespace passId
{
inline constexpr const char* SkyboxPass = "skyboxPass";
} // namespace passId

struct SkyboxModuleData
{
    graphics::VulkanPipeline skyboxPipeline{};
    ModuleRenderingInfo renderingInfo;
};

class SkyboxModule : public BaseModule
{
  public:
    explicit SkyboxModule( FrameGraph* graph,
                           graphics::VulkanContext* ctx,
                           VkExtent2D extent,
                           graphics::FrameInFlightVulkanDescriptor* cameraDescriptor,
                           glm::vec4 clearColor );

    explicit SkyboxModule( FrameGraph* graph,
                           graphics::VulkanContext* ctx,
                           VkExtent2D extent,
                           graphics::FrameInFlightVulkanDescriptor* cameraDescriptor );

    ~SkyboxModule();

    auto setExtent( VkExtent2D extent ) -> void override;

    /**
     * @brief Create the pipelines from the module data and register
     * render passes
     * @return
     */
    auto registerPasses() -> void override;

    /**
     * @brief Initialize and allocate all the necessary resources for using this module to render
     * @param extent Extent of the images that this pass is going to render to
     * @return
     */
    auto createModuleResources( VkExtent2D extent ) -> void override;

    auto recreate( VkExtent2D extent ) -> void override;

  protected:
    auto destroyModuleResources() -> void override;
    auto registerSkyboxPass() -> void;

  private:
    graphics::FrameInFlightVulkanDescriptor* m_cameraDescriptor;
    graphics::VulkanContext* m_vkCtx;
    rendering::FrameGraph* m_graph;
    VkExtent2D m_extent{};
};
} // namespace rendering