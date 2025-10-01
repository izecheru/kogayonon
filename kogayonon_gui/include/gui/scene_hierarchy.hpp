#include <entt/entt.hpp>
#include "gui/imgui_window.hpp"

namespace kogayonon_core
{
class Scene;
class Entity;
struct TextureComponent;
class KeyPressedEvent;
class MouseClickedEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
struct PopUp
{
  bool draw{ false };
  int x{ 0 };
  int y{ 0 };
};

class SceneHierarchyWindow : public ImGuiWindow
{
public:
  explicit SceneHierarchyWindow( std::string name );
  ~SceneHierarchyWindow() = default;

  void onKeyPressed( const kogayonon_core::KeyPressedEvent& e );

  void draw() override;

  /**
   * @brief Draws a context menu for the item we provide, we need the item id
   * @param id Id of the element we draw the context menu for
   * @param name Just a test parameter for the moment
   */
  void drawItemContexMenu( const std::string& itemId, kogayonon_core::Entity& ent ) const;

  void drawContextMenu();

private:
  int m_selectedIndex;
  std::weak_ptr<kogayonon_core::Scene> m_pCurrentScene;
  PopUp m_popUp;
};
} // namespace kogayonon_gui