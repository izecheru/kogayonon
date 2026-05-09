#pragma once
#include "imgui_base.hpp"
#include "precompiled/pch.hpp"
#include <ImGuizmo.h>
#include <SDL2/SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

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
  TRANSLATE,
  SCALE,
  ROTATE
};

enum class AxisLock
{
  NONE,
  X,
  Y,
  Z
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

private: // funcs
  void drawToolbar();
  auto getGuizmoOp() -> ImGuizmo::OPERATION;

private:
  // Events
  void onSelectedEntity( const core::SelectEntityEvent& e );
  // void onMouseMoved( const kogayonon_core::MouseMovedEvent& e );
  // void onMouseClicked( const kogayonon_core::MouseClickedEvent& e );
  void onKeyPressed( const core::KeyPressedEvent& e );
  // void onMouseScrolled( const kogayonon_core::MouseScrolledEvent& e );

private:
  SDL_Window* m_mainWindow;
  ViewportSpec m_spec;
  entt::entity m_selectedEntity;

  ImGuizmo::OPERATION m_guizmoOp;
  bool m_guizmoEnabled;
  GuizmoMode m_guizmoMode;
  AxisLock m_guizmoAxisLock;
};
} // namespace gui