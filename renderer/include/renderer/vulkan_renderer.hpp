#pragma once
#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_descriptor.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "precompiled/pch.hpp"
#include "utilities/shader_compiler/shader_compiler.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#define MAX_TEXTURE_NUM 1000

struct SDL_Window;

struct CameraUBO
{
  glm::mat4 view;
  glm::mat4 proj;
};

namespace core
{
class MouseClickedEvent;
class SelectEntityEvent;
} // namespace core

namespace graphics
{
struct VulkanContext;
}

namespace gui

{
class VulkanImguiRenderer;
}

namespace rendering
{

struct VulkanViewport
{
  VkImage image;
  VkImageView imageView;
  VmaAllocation allocation;

  VkImage depthImage;
  VkImageView depthView;
  VmaAllocation depthAllocation;
};

class VulkanRenderer
{
public:
  explicit VulkanRenderer( graphics::VulkanContext* pCtx, SDL_Window* window );

  ~VulkanRenderer();

  /**
   * @brief Rendering function, passes should be different functions
   */
  void render();

  auto getViewport() -> VulkanViewport&;

  /**
   * @brief Initialize ImGui UI renderer
   */
  void initImgui();

private: // Events
  void onMouseClicked( core::MouseClickedEvent& e );
  void onEntitySelect( core::SelectEntityEvent& e );

private: // funcs
  void geometryPass( VkCommandBuffer& cmd );
  void pickingPass( VkCommandBuffer& cmd );
  void imguiPass( VkCommandBuffer& cmd );

  void createViewport();
  void createPickingViewport();

  void createPickingBuffers();

  void createCameraBuffers();
  void updateCameraBuffer();
  void createCameraDescriptorSetLayout();
  void createCameraDescriptorSet();

  void createPipeline( const graphics::VulkanPipelineSpec& spec );

private:
  bool m_assetManagerInit{ false };
  // this should be tied to scene or smth cause we can create a camera entity
  CameraUBO m_cameraUbo;
  graphics::FrameInFlightVulkanBuffer m_cameraBuffers;
  graphics::BufferedVulkanDescriptor m_cameraDescriptor;

  std::map<graphics::PipelineType, graphics::VulkanPipeline> m_pipelines;

  graphics::VulkanContext* m_pVkContext{ nullptr };
  std::shared_ptr<gui::VulkanImguiRenderer> m_pImguiRenderer;

  VulkanViewport m_viewport;
  VulkanViewport m_pickingViewport;

  graphics::FrameInFlightVulkanBuffer m_pickingBuffer;

  SDL_Window* m_wnd;
  utilities::ShaderCompiler m_shaderCompiler;

  glm::ivec2 m_mouseCoord;

  entt::entity m_selectedEntity;
};
} // namespace rendering