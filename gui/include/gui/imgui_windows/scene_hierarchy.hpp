#include <vulkan/vulkan.h>
#include <entt/entt.hpp>
#include "gui/imgui_windows/imgui_base.hpp"

namespace core
{
class Scene;
class Entity;

class SelectEntityEvent;
class DeleteEntityEvent;
class KeyPressedEvent;
class MouseClickedEvent;
} // namespace core

struct SceneHierarchySpec
{
    std::unordered_map<std::string, ImFont*>* fonts;

    VkDescriptorSet cubeIcon;
};

namespace gui
{
class SceneHierarchy : public ImGuiWindow
{
  public:
    explicit SceneHierarchy( const std::string& name, const SceneHierarchySpec& spec );
    ~SceneHierarchy() = default;

    auto render() -> void override;
    auto getSpec() -> SceneHierarchySpec&;

  private:
    auto drawContextMenu() -> void;

    /**
     * @brief Draws a context menu for the item we provide, we need the item id
     * @param id Id of the element we draw the context menu for
     * @param name Just a test parameter for the moment
     */
    auto drawItemContexMenu( const std::string& itemId, entt::entity ent ) -> void;

    auto onKeyPressed( const core::KeyPressedEvent& e ) -> void;

  private:
    SceneHierarchySpec m_spec;
};
} // namespace gui