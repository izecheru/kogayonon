#include <entt/entt.hpp>
#include "gui/imgui_window.hpp"

namespace kogayonon_core
{
class Scene;
class Entity;
class SelectEntityInViewportEvent;
struct TextureComponent;
class KeyPressedEvent;
class MouseClickedEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class SceneHierarchyWindow : public ImGuiWindow
{
public:
  explicit SceneHierarchyWindow( std::string name );
  ~SceneHierarchyWindow() = default;

  void onEntitySelectInViewport( const kogayonon_core::SelectEntityInViewportEvent& e );

  void onKeyPressed( const kogayonon_core::KeyPressedEvent& e );

  void draw() override;

  /**
   * @brief Draws a context menu for the item we provide, we need the item id
   * @param id Id of the element we draw the context menu for
   * @param name Just a test parameter for the moment
   */
  void drawItemContexMenu( const std::string& itemId, kogayonon_core::Entity& ent );

  void drawContextMenu();

private:
  void duplicateEntity();

private:
  entt::entity m_selectedEntity;
};
} // namespace kogayonon_gui