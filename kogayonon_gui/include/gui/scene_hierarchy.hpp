#include "gui/imgui_window.hpp"

namespace kogayonon_core
{
class Scene;
struct TextureComponent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class SceneHierarchyWindow : public ImGuiWindow
{
public:
  explicit SceneHierarchyWindow( std::string name );
  ~SceneHierarchyWindow() = default;

  void draw() override;
  void drawTextureTooltip( kogayonon_core::TextureComponent* textureComp, ImVec2 size );

private:
  std::weak_ptr<kogayonon_core::Scene> m_pCurrentScene;
};
} // namespace kogayonon_gui