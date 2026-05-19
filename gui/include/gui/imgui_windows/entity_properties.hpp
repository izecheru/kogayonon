#pragma once
#include "gui/imgui_windows/imgui_base.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

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
  void contextMenu();

  void renderMesh();

  void renderCamera();
  void renderCameraFov( bool& changed, float& fov );
  void renderCameraNear( bool& changed, float& camNear );
  void renderCameraFar( bool& changed, float& camFar );

  /**
   * @brief Render all the details about the transform component
   */
  void renderTransform();

  void renderTranslation( bool& translationChanged, glm::vec3& translation );
  void renderScale( bool& scaleChanged, glm::vec3& scale );
  void renderRotation( bool& rotationChanged, glm::vec3& rotation );

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
