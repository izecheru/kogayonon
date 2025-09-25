#pragma once
#include <entt/entt.hpp>
#include "gui/imgui_window.hpp"

namespace kogayonon_core
{
class Scene;
class SelectEntityEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class EntityPropertiesWindow : public ImGuiWindow
{
public:
  explicit EntityPropertiesWindow( std::string name );
  ~EntityPropertiesWindow() = default;

  void draw() override;

  bool onEnitySelect( const kogayonon_core::SelectEntityEvent& e );

private:
  entt::entity m_entity;
  std::weak_ptr<kogayonon_core::Scene> m_pCurrentScene;
};
} // namespace kogayonon_gui