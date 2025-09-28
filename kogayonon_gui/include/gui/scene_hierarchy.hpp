#include <entt/entt.hpp>
#include "gui/imgui_window.hpp"

namespace kogayonon_core
{
class Scene;
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
   * @brief Draws a preview of a texture and some information about it
   * @param textureComp The texture component we need to get info about the texture
   * @param size Size of the tooltip
   */
  void drawTextureTooltip( kogayonon_core::TextureComponent* textureComp, ImVec2 size );

  /**
   * @brief Draws a context menu for the item we provide, we need the item id
   * @param id Id of the element we draw the context menu for
   * @param name Just a test parameter for the moment
   */
  void drawItemContexMenu( const std::string& itemId, std::string name );

  void drawContextMenu();

private:
  int m_selectedIndex;
  std::weak_ptr<kogayonon_core::Scene> m_pCurrentScene;
  PopUp m_popUp;
};
} // namespace kogayonon_gui