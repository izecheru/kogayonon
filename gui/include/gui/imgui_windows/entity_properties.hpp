#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
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

    auto render() -> void;
    auto getSpec() -> EntityPropertiesSpec&;

  private:
    auto contextMenu() -> void;

    auto renderMesh() -> void;

    auto renderCamera() -> void;
    auto renderCameraFov( bool& changed, float& fov ) -> void;
    auto renderCameraNear( bool& changed, float& camNear ) -> void;
    auto renderCameraFar( bool& changed, float& camFar ) -> void;

    auto renderRigidbody() -> void;

    /**
     * @brief Render all the details about the transform component
     */
    auto renderTransform() -> void;

    auto renderTranslation( bool& translationChanged, glm::vec3& translation ) -> void;
    auto renderScale( bool& scaleChanged, glm::vec3& scale ) -> void;
    auto renderRotation( bool& rotationChanged, glm::vec3& rotation ) -> void;

    /**
     * @brief Renders details about the IdentifierComponent of the currently selected entity
     */
    auto renderIdentification() -> void;

    /**
     * @brief Renders the X, Y, Z for the translation, rotation and scale
     * @param axis Label for the axis
     * @param color Color of the text background
     */
    auto renderColoredAxis( const std::string& axis, const ImU32& color ) -> void;

  private:
    EntityPropertiesSpec m_spec;
};
} // namespace gui
