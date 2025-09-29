#pragma once
#include <entt/entt.hpp>
#include "gui/imgui_window.hpp"

namespace kogayonon_core
{
class Scene;
class SelectEntityEvent;
struct TextureComponent;
class Entity;
struct ModelComponent;
struct TransformComponent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class EntityPropertiesWindow : public ImGuiWindow
{
public:
  explicit EntityPropertiesWindow( std::string name );
  ~EntityPropertiesWindow() = default;

  void draw() override;

  void onEntitySelect( const kogayonon_core::SelectEntityEvent& e );

private:
  void drawEnttProperties( std::shared_ptr<kogayonon_core::Scene> scene );

  void drawTextureComponent( kogayonon_core::Entity& ent );
  void drawModelComponent( kogayonon_core::Entity& ent );
  void drawTransformComponent( kogayonon_core::Entity& ent );

private:
  entt::entity m_entity;
  std::weak_ptr<kogayonon_core::Scene> m_pCurrentScene;
};
} // namespace kogayonon_gui