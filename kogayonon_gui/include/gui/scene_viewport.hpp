#pragma once
#include <SDL2/SDL.h>
#include <entt/entt.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include "imgui_window.hpp"
#include "rendering/opengl_framebuffer.hpp"

namespace kogayonon_resources
{

} // namespace kogayonon_resources

namespace kogayonon_rendering
{
class Camera;
} // namespace kogayonon_rendering

namespace kogayonon_core
{
class Scene;
class RenderingSystem;

class SaveSceneEvent;
class SelectEntityEvent;
class KeyPressedEvent;
class MouseMovedEvent;
class MouseScrolledEvent;
class MouseClickedEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
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

class SceneViewportWindow : public ImGuiWindow
{
public:
  /**
   * @brief The Viewport where we draw our scene
   * @param mainWindow This is injected for the camera movement (SDL_MouseRelativeMode)
   * @param name The name of the ImGuiWindow
   * @param playTexture The play button texture ID
   * @param stopTexture The stop button texture ID
   */
  explicit SceneViewportWindow( SDL_Window* mainWindow, std::string name, unsigned int playTexture,
                                unsigned int stopTexture );
  ~SceneViewportWindow() = default;

  void draw() override;

  void drawScene();
  void drawPickingScene();

  // Events
  void onSelectedEntity( const kogayonon_core::SelectEntityEvent& e );
  void onMouseMoved( const kogayonon_core::MouseMovedEvent& e );
  void onMouseClicked( const kogayonon_core::MouseClickedEvent& e );
  void onKeyPressed( const kogayonon_core::KeyPressedEvent& e );
  void onMouseScrolled( const kogayonon_core::MouseScrolledEvent& e );
  void onSaveScene( const kogayonon_core::SaveSceneEvent& e );

private:
  entt::entity m_selectedEntity;
  unsigned int m_playTextureId;
  unsigned int m_stopTextureId;
  SDL_Window* m_mainWindow;

  // drawing framebuffer
  kogayonon_rendering::OpenGLFramebuffer m_frameBuffer;
  // picking framebuffer
  kogayonon_rendering::OpenGLFramebuffer m_pickingFrameBuffer;

  std::unique_ptr<kogayonon_core::RenderingSystem> m_pRenderingSystem;
  // perspective camera, should make it an entity and add it to registry
  std::unique_ptr<kogayonon_rendering::Camera> m_pCamera;
  GizmoMode m_gizmoMode;
};
} // namespace kogayonon_gui