#include "gui/imgui_window.h"

namespace kogayonon_core
{
class Scene;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class SceneHierarchyWindow : public ImGuiWindow
{
  public:
    explicit SceneHierarchyWindow( std::string name );
    ~SceneHierarchyWindow() = default;

    void draw() override;

  private:
    std::weak_ptr<kogayonon_core::Scene> m_pCurrentScene;
};
} // namespace kogayonon_gui