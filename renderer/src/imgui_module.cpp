#include "renderer/modules/imgui_module.hpp"
#include "graphics/vulkan_context.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/frame_graph.hpp"
#include "utilities/tracy_utils/tracy_utils.hpp"
#include "renderer/modules/geometry_module.hpp"

rendering::ImGuiModule::ImGuiModule( FrameGraph* graph,
                                     graphics::VulkanContext* vkCtx,
                                     gui::VulkanImguiRenderer* imguiRenderer )
    : m_vkCtx{ vkCtx }
    , m_imguiRenderer{ imguiRenderer }
    , m_graph{ graph }
{
  registerPasses();
}

rendering::ImGuiModule::~ImGuiModule()
{
}

void rendering::ImGuiModule::registerPasses()
{
  registerImGuiPass();
}

void rendering::ImGuiModule::registerImGuiPass()
{
  Blackboard* blackboard = m_graph->getBlackboard();
  blackboard->addToStorage<ImGuiModuleData>();

  m_graph->addNode(
    std::string{ passId::ImGui },
    []( NodeBuilder& b, Blackboard* blackboard ) -> void {
      GeometryModuleData& geometryData = blackboard->get<GeometryModuleData>();
      b.read( geometryData.color, FGResourceType::Color );
    },
    [=]( VkCommandBuffer cmdBuffer ) {
      TracyVkZone( m_vkCtx->tracyContext->getCtx(), cmdBuffer, passId::ImGui );
      ImGuiModuleData& imguiData = blackboard->get<ImGuiModuleData>();

      imguiData.renderingAttachment =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = m_vkCtx->swapchain->getImageAtAquiredIndex().vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { { 1.f, 1.f, 1.f, 1.0f } } };

      imguiData.renderingInfo = VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { { 0, 0 }, m_vkCtx->swapchain->getSwapchainExtent() },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &imguiData.renderingAttachment,
        .pDepthAttachment = nullptr,
      };

      m_vkCtx->swapchain->beginRendering( imguiData.renderingInfo );
      m_imguiRenderer->render();
      m_imguiRenderer->renderDrawData( cmdBuffer );
      m_vkCtx->swapchain->endRendering();
    } );
}

auto rendering::ImGuiModule::setViewport() -> void
{
  GeometryModuleData& geometryData = m_graph->getBlackboard()->get<GeometryModuleData>();
  KASSERT( geometryData.color->vulkanImage.vkImageView != VK_NULL_HANDLE );
  m_imguiRenderer->setViewport( geometryData.color->vulkanImage.vkImageView );
}
