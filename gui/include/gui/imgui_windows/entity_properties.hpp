#pragma once
#include <entt/entt.hpp>
#include "gui/imgui_windows/imgui_base.hpp"

namespace core
{
class SelectEntityEvent;
struct TransformComponent;
} // namespace core

namespace gui
{
struct EntityPropertiesSpec
{
  std::unordered_map<std::string, ImFont*>* fonts;
};

class EntityProperties : public ImGuiWindow
{
public:
  explicit EntityProperties( const std::string& name, const EntityPropertiesSpec& spec );
  ~EntityProperties() = default;

  void render();

private:
  // EVENTS
  void onSelectEntity( const core::SelectEntityEvent& e );
  //------------------

private:
  /**
   * @brief Renders all the details about the transform component
   */
  void renderTransform();
  void renderTransformTranslation( bool& translationChanged, core::TransformComponent* pTransform );
  void renderTransformScale( bool& scaleChanged, core::TransformComponent* pTransform );
  void renderTransformRotation( bool& rotationChanged, core::TransformComponent* pTransform );

  /**
   * @brief Renders details about the IdentifierComponent of the currently selected entity
   */
  void renderIdentification();

  /**
   * @brief Renders the X, Y, Z for the translation, rotation and scale
   * @param axis Label for the axis
   * @param color Color of the text background
   */
  void renderColoredAxis( const std::string& axis, const ImU32& color );

private:
  entt::entity m_selectedEntity;
  EntityPropertiesSpec m_spec;
};
} // namespace gui
