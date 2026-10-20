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

    /**
     * @brief Render details about the cubemap component
     *
     */
    auto renderCubemap() -> void;

    /**
     * @brief Render details about the model component
     *
     */
    auto renderModel() -> void;

    /**
     * @brief Render camera related info
     *
     */
    auto renderCamera() -> void;

    /**
     * @brief Render details about the rigidbody component
     *
     */
    auto renderRigidbody() -> void;

    /**
     * @brief Render all the details about the transform component
     */
    auto renderTransform() -> void;

    // TODO thos can be simplified
    auto renderTranslation( bool& translationChanged, glm::vec3& translation ) -> void;
    auto renderScale( bool& scaleChanged, glm::vec3& scale ) -> void;
    auto renderRotation( bool& rotationChanged, glm::vec3& rotation ) -> void;

    /**
     * @brief Renders details about the IdentifierComponent of the currently selected entity
     *
     */
    auto renderIdentification() -> void;

    /**
     * @brief Render a colored square with the axis text inside of it
     *
     * @param axis The axis, eg. X
     * @param color Color of the axis square
     */
    auto renderColoredAxis( const std::string& axis, const ImU32& color ) -> void;

  private:
    EntityPropertiesSpec m_spec;
};
} // namespace gui
