#pragma once
#include <SDL2/SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "imgui_base.hpp"
#include "precompiled/pch.hpp"

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

enum class GizmoMode
{
  SCALE,
  SCALE_X,
  SCALE_Y,
  SCALE_Z,

  ROTATE,
  ROTATE_X,
  ROTATE_Y,
  ROTATE_Z,

  TRANSLATE,
  TRANSLATE_X,
  TRANSLATE_Y,
  TRANSLATE_Z
};

enum class RenderMode
{
  Geometry,
  GeometryAndLights,
  Depth
};

struct ViewportSpec
{
  std::unordered_map<std::string, ImFont*>* fonts;

  VkDescriptorSet renderModeIcon;
  VkDescriptorSet playIcon;
  VkDescriptorSet stopIcon;

  VkImageView* pViewportTexture;
  VkSampler* pSampler;
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

  void drawToolbar();

  // Events
  // void onSelectedEntity( const kogayonon_core::SelectEntityEvent& e );
  // void onMouseMoved( const kogayonon_core::MouseMovedEvent& e );
  // void onMouseClicked( const kogayonon_core::MouseClickedEvent& e );
  // void onKeyPressed( const kogayonon_core::KeyPressedEvent& e );
  // void onMouseScrolled( const kogayonon_core::MouseScrolledEvent& e );

private:
  SDL_Window* m_mainWindow;
  GizmoMode m_gizmoMode;
  bool m_gizmoEnabled{ false };
  ViewportSpec m_spec;
};
} // namespace gui