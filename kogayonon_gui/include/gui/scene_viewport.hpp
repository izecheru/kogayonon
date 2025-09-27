#pragma once
#include <SDL2/SDL.h>
#include <entt/entt.hpp>
#include <filesystem>
#include "imgui_window.hpp"

namespace kogayonon_rendering
{
class FrameBuffer;
class Camera;
} // namespace kogayonon_rendering

namespace kogayonon_core
{
class Scene;
class SelectEntityEvent;
class KeyPressedEvent;
class MouseMovedEvent;
class MouseScrolledEvent;
class MouseClickedEvent;
class RenderingSystem;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class SceneViewportWindow : public ImGuiWindow
{
public:
  explicit SceneViewportWindow( SDL_Window* mainWindow, std::string name,
                                std::weak_ptr<kogayonon_rendering::FrameBuffer> frameBuffer, unsigned int playTexture,
                                unsigned int stopTexture );
  ~SceneViewportWindow() = default;

  void draw() override;
  std::weak_ptr<kogayonon_rendering::FrameBuffer> getFrameBuffer();

  void onSelectedEntity( const kogayonon_core::SelectEntityEvent& e );
  void onMouseMoved( const kogayonon_core::MouseMovedEvent& e );
  void onMouseClicked( const kogayonon_core::MouseClickedEvent& e );
  void onKeyPressed( const kogayonon_core::KeyPressedEvent& e );
  void onMouseScrolled( const kogayonon_core::MouseScrolledEvent& e );

private:
  /**
   * @brief Processes the payload from the Assets window
   * @param payload The payload we drag and dropped on the viewport
   */
  void manageAssetsPayload( const ImGuiPayload* payload ) const;

private:
  entt::entity m_selectedEntity;
  std::weak_ptr<kogayonon_rendering::FrameBuffer> m_pFrameBuffer;
  unsigned int m_playTextureId;
  unsigned int m_stopTextureId;
  SDL_Window* m_mainWindow;
  std::unique_ptr<kogayonon_core::RenderingSystem> m_pRenderingSystem;
  std::unique_ptr<kogayonon_rendering::Camera> m_pCamera;
};
} // namespace kogayonon_gui