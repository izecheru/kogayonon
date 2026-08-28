#pragma once
#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_descriptor.hpp"
#include "graphics/vulkan_image.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "precompiled/pch.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#define MAX_TEXTURE_NUM 1000

struct SDL_Window;

namespace rendering
{
class FrameGraph;
class GeometryModule;
class ImGuiModule;
class PickingModule;
class PrepassModule;
} // namespace rendering

enum DescriptorSetNum
{
  SET_0 = 0,
  SET_1 = 1,
  SET_2 = 2
};

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

  /**
   * @brief Initialize ImGui UI renderer
   */
  auto initImgui() -> void;

protected:
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
  graphics::VulkanDescriptor m_cameraDescriptor;
  std::shared_ptr<gui::VulkanImguiRenderer> m_pImguiRenderer;
  glm::ivec2 m_mouseCoords;

  SDL_Window* m_wnd;

  std::unique_ptr<rendering::GeometryModule> m_geometryModule;
  std::unique_ptr<rendering::ImGuiModule> m_imguiModule;
  std::unique_ptr<rendering::PickingModule> m_pickingModule;
  std::unique_ptr<rendering::PrepassModule> m_prepassModule;
};
} // namespace rendering