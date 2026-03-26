#include <entt/entt.hpp>
#include "gui/imgui_windows/imgui_base.hpp"

namespace core
{
struct TextureComponent;

class Scene;
class Entity;

class SelectEntityEvent;
class KeyPressedEvent;
class MouseClickedEvent;
} // namespace core

struct SceneHierarchySpec
{
};

namespace gui
{
class SceneHierarchy : public ImGuiWindow
{
public:
  explicit SceneHierarchy( const std::string& name, const SceneHierarchySpec& spec );
  ~SceneHierarchy() = default;

  void onEntitySelect( const core::SelectEntityEvent& e );
  void onKeyPressed( const core::KeyPressedEvent& e );

  void render() override;

  /**
   * @brief Draws a context menu for the item we provide, we need the item id
   * @param id Id of the element we draw the context menu for
   * @param name Just a test parameter for the moment
   */
  void drawItemContexMenu( const std::string& itemId, entt::entity ent );

  void drawContextMenu();

private:
  void duplicateEntity( core::Entity& ent );
  void deleteEntity( core::Entity& ent );

private:
  entt::entity m_selectedEntity;
  SceneHierarchySpec m_spec;
};
} // namespace gui