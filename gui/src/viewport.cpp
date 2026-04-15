#include "gui/imgui_windows/viewport.hpp"
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include "gui/utils/imgui_utils.hpp"
#include "utilities/fonts/materialdesign.hpp"

gui::Viewport::Viewport( SDL_Window* mainWindow, const std::string& name, const ViewportSpec& spec )
    : ImGuiWindow{ name, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar }
    , m_spec{ spec }
    , m_mainWindow{ mainWindow }
    , m_gizmoMode{ gui::GizmoMode::ROTATE }
    , m_gizmoEnabled{ false }
{
}

void gui::Viewport::render()
{
  ImGuiWindowClass windowClass;
  windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

  ImGui::SetNextWindowClass( &windowClass );

  if ( !begin() )
    return;

  // TODO(kogayonon) draw the scene here
  auto viewportPanelSize = ImGui::GetContentRegionAvail();

  ImGui::BeginGroup();
  static auto viewportDescriptorSet =
    ImGui_ImplVulkan_AddTexture( *m_spec.pSampler, *m_spec.pViewportTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
  ImGui::Image( viewportDescriptorSet, viewportPanelSize );

  drawToolbar();
  ImGui::EndGroup();

  ImGui::End();
}

void gui::Viewport::drawToolbar()
{
  auto& style = ImGui::GetStyle();

  // WARNING thake this into account when mouse picking
  ImGui::SetCursorPos( { 20.0f, 20.0f } );

  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, { 8.0f, 8.0f } );
  ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 10.0f );
  ImGui::PushStyleColor( ImGuiCol_ChildBg, { 0.15f, 0.15f, 0.15f, 0.75f } );

  constexpr int buttonCount = 3;
  static float toolbarWidth =
    style.WindowPadding.x * 2.0f + ( 14.0f * buttonCount ) + ( style.ItemSpacing.x * buttonCount ) + ( 2.0f * 5.0f );

  if ( ImGui::BeginChild( "Toolbar",
                          { toolbarWidth, 25.0f },
                          false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
  {

    ImGui::PushStyleColor( ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f } );
    ImGui::PushStyleColor( ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 0.0f } );

    ImGui::SetCursorPos( { 5.0f, 2.5f } );
    if ( ImGui::ImageButton( "##renderMode", m_spec.renderModeIcon, { 14.0f, 14.0f } ) )
    {
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "##stopButton", m_spec.stopIcon, { 14.0f, 14.0f } ) )
    {
      // kogayonon_physics::NvidiaPhysx::getInstance().switchState( false );
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "##startButton", m_spec.playIcon, { 14.0f, 14.0f } ) )
    {
      // kogayonon_physics::NvidiaPhysx::getInstance().switchState( true );
    }

    ImGui::PopStyleColor( 2 );
    ImGui::EndChild();
  }

  ImGui::PopStyleColor();
  ImGui::PopStyleVar( 2 );
}
