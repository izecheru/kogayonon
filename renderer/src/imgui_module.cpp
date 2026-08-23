#include "renderer/modules/imgui_module.hpp"
#include "graphics/vulkan_context.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/frame_graph.hpp"

// TODO maybe move module data structs to a separate file each
#include "renderer/modules/geometry_module.hpp"

rendering::ImGuiModule::ImGuiModule( graphics::VulkanContext* vkCtx,
                                     FrameGraph* graph,
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
  m_graph->addNode(
    std::string{ passId::ImGui },
    [=]( NodeBuilder& b, Blackboard* blackboard ) -> void {
      blackboard->addToStorage<ImGuiModuleData>();
      GeometryModuleData& geometryData{ m_graph->getBlackboard()->get<GeometryModuleData>() };
      b.read( geometryData.color, FGResourceType::Color );
    },
    [=]( VkCommandBuffer buffer ) {
      ImGuiModuleData& imguiData = blackboard->get<ImGuiModuleData>();

      imguiData.renderingAttachment =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = m_vkCtx->swapchain->getImageAtAquiredIndex().vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   // this clears the swapchain image
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

      rendering::GeometryModuleData& geometryData{ m_graph->getBlackboard()->get<GeometryModuleData>() };

      static bool first{ true };
      if ( first )
        m_imguiRenderer->setViewport( geometryData.color->vulkanImage.vkImageView );

      first = false;

      m_vkCtx->swapchain->beginRendering( imguiData.renderingInfo );
      m_imguiRenderer->render();
      m_imguiRenderer->present( buffer );
      m_vkCtx->swapchain->endRendering();
    } );
}
