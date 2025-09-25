#include <entt/entt.hpp>
#include "gui/imgui_window.hpp"

namespace kogayonon_core
{
class Scene;
struct TextureComponent;
class KeyPressedEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class SceneHierarchyWindow : public ImGuiWindow
{
public:
  explicit SceneHierarchyWindow( std::string name );
  ~SceneHierarchyWindow() = default;

  void onKeyPressed( const kogayonon_core::KeyPressedEvent& e );

  void draw() override;
  void drawTextureTooltip( kogayonon_core::TextureComponent* textureComp, ImVec2 size );

private:
  int m_selectedIndex;
  std::weak_ptr<kogayonon_core::Scene> m_pCurrentScene;
};
} // namespace kogayonon_gui