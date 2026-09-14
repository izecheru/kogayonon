#pragma once
#include <vulkan/vulkan.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_descriptor.hpp"
#include "graphics/vulkan_image.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "precompiled/pch.hpp"

#define MAX_TEXTURE_NUM 1000

struct SDL_Window;

namespace rendering
{
class FrameGraph;
class GeometryModule;
class ImGuiModule;
class PickingModule;
class PrepassModule;
class ShadowmapModule;
} // namespace rendering

namespace core
{
class Entity;
class MouseClickedEvent;
class SelectEntityEvent;
class ImGuiWindowResizeEvent;
} // namespace core

namespace graphics
{
struct VulkanContext;
} // namespace graphics

namespace gui
{
class VulkanImguiRenderer;
}

namespace rendering
{
class VulkanRenderer
{
public:
  explicit VulkanRenderer( graphics::VulkanContext* pCtx, SDL_Window* window );
  ~VulkanRenderer();

  auto render() -> void;
  auto presentToScreen() -> void;
  auto onUpdate() -> void;

protected:
  /**
   * @brief Initialize ImGui UI renderer
   */
  auto initImgui() -> void;
  auto initModules() -> void;

  auto onMouseClicked( const core::MouseClickedEvent& e ) -> void;

private:
  auto createCameraBuffers() -> void;
  auto updateCameraBuffer() -> void;
  auto createCameraDescriptorSetLayout() -> void;
  auto createCameraDescriptorSet() -> void;

private:
  graphics::VulkanContext* m_vkCtx{ nullptr };
  std::unique_ptr<FrameGraph> m_frameGraph;
  graphics::FrameInFlightVulkanBuffer m_cameraBuffers;
  graphics::FrameInFlightVulkanDescriptor m_cameraDescriptor;
  std::shared_ptr<gui::VulkanImguiRenderer> m_pImguiRenderer;
  glm::ivec2 m_mouseCoords;
  VkExtent2D m_extent;

  SDL_Window* m_wnd;

  std::unique_ptr<rendering::GeometryModule> m_geometryModule;
  std::unique_ptr<rendering::ImGuiModule> m_imguiModule;
  std::unique_ptr<rendering::PickingModule> m_pickingModule;
  std::unique_ptr<rendering::PrepassModule> m_prepassModule;
  std::unique_ptr<rendering::ShadowmapModule> m_shadowmapModule;
};
} // namespace rendering