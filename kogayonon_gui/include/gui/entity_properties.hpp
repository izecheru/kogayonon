#pragma once
#include <entt/entt.hpp>
#include <mutex>
#include "gui/imgui_window.hpp"

namespace kogayonon_resources
{
class Texture;
}

namespace kogayonon_core
{
class Scene;
class SelectEntityEvent;
class SelectEntityInViewportEvent;
struct TextureComponent;
struct ModelComponent;
struct TransformComponent;
class Entity;
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
  void onSelectEntityInViewport( const kogayonon_core::SelectEntityInViewportEvent& e );

private:
  void drawEnttProperties( std::shared_ptr<kogayonon_core::Scene> scene );
  void drawTextureContextMenu( std::vector<std::weak_ptr<kogayonon_resources::Texture>>& textures, int index ) const;
  void drawTextureComponent( kogayonon_core::Entity& ent ) const;
  void drawModelComponent( kogayonon_core::Entity& ent );
  void drawTransformComponent( kogayonon_core::Entity& ent ) const;

  void manageModelPayload( const ImGuiPayload* payload );
  void manageTexturePayload( const ImGuiPayload* payload ) const;

private:
  entt::entity m_entity;
};
} // namespace kogayonon_gui