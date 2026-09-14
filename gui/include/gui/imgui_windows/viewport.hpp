#pragma once
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include "imgui_base.hpp"
#include "precompiled/pch.hpp"
#include <ImGuizmo.h>

namespace core
{

class Scene;
class RenderingSystem;

class SelectEntityEvent;
class KeyPressedEvent;
class MouseMovedEvent;
class MouseScrolledEvent;
class MouseClickedEvent;
} // namespace core

namespace gui
{

enum class GuizmoMode
{
  Translate,
  Scale,
  Rotate
};
enum class AxisLock
{
  None,
  X_axis,
  Y_axis,
  Z_axis
};

struct ViewportSpec
{
  std::unordered_map<std::string, ImFont*>* fonts;

  VkDescriptorSet renderModeIcon;
  VkDescriptorSet playIcon;
  VkDescriptorSet stopIcon;

  VkImageView& viewportTexture;
  VkSampler& sampler;
};

class Viewport : public ImGuiWindow
{
public:
  /**
   * @brief The Viewport where we draw our scene
   * @param mainWindow This is injected for the camera movement (SDL_MouseRelativeMode)
   * @param name The name of the ImGuiWindow
   */
  explicit Viewport( SDL_Window* mainWindow, const std::string& name, const ViewportSpec& spec );
  ~Viewport() = default;

  void render() override;

  /**
   * @brief Removes the current viewport descriptor and assigns it to imageView
   * @param imageView The image view of the texture we rendered the scene to
   * @return void
   */
  auto setViewport( VkImageView imageView ) -> void;

private:
  void drawToolbar();
  void drawEntityMenu();
  auto getGuizmoOp() -> ImGuizmo::OPERATION;

private:
  void onKeyPressed( const core::KeyPressedEvent& e );

private:
  SDL_Window* m_mainWindow;
  ViewportSpec m_spec;

  ImGuizmo::OPERATION m_guizmoOp;
  bool m_guizmoEnabled;
  GuizmoMode m_guizmoMode;
  AxisLock m_guizmoAxisLock;
  VkDescriptorSet m_viewportDescriptor{ VK_NULL_HANDLE };

  glm::vec2 m_mouseCoords;
  bool m_entityMenu;
};
} // namespace gui