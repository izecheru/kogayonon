#pragma once
#include <entt/entt.hpp>
#include "gui/imgui_window.hpp"

namespace kogayonon_resources
{
class Texture;
struct PointLight;
} // namespace kogayonon_resources

namespace kogayonon_core
{
class Scene;
class SelectEntityEvent;
class SelectEntityInViewportEvent;
struct TextureComponent;
struct MeshComponent;
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
  void drawTextureContextMenu( std::vector<kogayonon_resources::Texture*>& textures, int index ) const;
  void drawTextureComponent( kogayonon_core::Entity& ent ) const;
  void drawMeshComponent( kogayonon_core::Entity& ent );
  void drawTransformComponent( kogayonon_core::Entity& ent ) const;
  void drawPointLightComponent( kogayonon_core::Entity& ent ) const;
  void drawDirectionalLightComponent( kogayonon_core::Entity& ent ) const;
  void drawDynamicRigidbodyComponent( kogayonon_core::Entity& ent ) const;
  void drawStaticRigidbodyComponent( kogayonon_core::Entity& ent ) const;

  void drawRigidbodyMenu( kogayonon_core::Entity& ent );
  void drawLightMenu( kogayonon_core::Entity& ent );

  void manageModelPayload( const ImGuiPayload* payload );
  void manageTexturePayload( const ImGuiPayload* payload ) const;

private:
  entt::entity m_selectedEntity;
};
} // namespace kogayonon_gui