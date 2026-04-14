#include "gui/imgui_windows/viewport.hpp"
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include "gui/utils/imgui_utils.hpp"
#include "utilities/fonts/materialdesign.hpp"

gui::Viewport::Viewport( SDL_Window* mainWindow, const std::string& name, const ViewportSpec& spec )
    : ImGuiWindow{ name }
    , m_spec{ spec }
    , m_mainWindow{ mainWindow }
    , m_gizmoMode{ gui::GizmoMode::ROTATE }
    , m_gizmoEnabled{ false }
{
}

void gui::Viewport::render()
{
  if ( !begin() )
    return;

  // TODO(kogayonon) draw the scene here
  static auto viewportDescriptorSet =
    ImGui_ImplVulkan_AddTexture( *m_spec.pSampler, *m_spec.pViewportTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
  ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
  ImGui::Image( viewportDescriptorSet, ImVec2{ viewportPanelSize.x, viewportPanelSize.y } );

  ImGui::End();
}

void gui::Viewport::drawToolbar()
{
  auto& style = ImGui::GetStyle();

  // WARNING thake this into account when mouse picking
  ImGui::SetCursorPos( ImVec2{ 10.0f, 10.0f } );

  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 8, 8 ) );
  ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 10.0f );
  ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4{ 0.15f, 0.15f, 0.15f, 0.75f } );

  constexpr int buttonCount = 3;
  static float toolbarWidth =
    style.WindowPadding.x * 2.0f + ( 14.0f * buttonCount ) + ( style.ItemSpacing.x * buttonCount ) + ( 2.0f * 5.0f );

  if ( ImGui::BeginChild( "Toolbar",
                          ImVec2{ toolbarWidth, 25.0f },
                          false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
  {
    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f } );

    ImGui::SetCursorPos( ImVec2{ 5.0f, 2.5f } );
    if ( ImGui::ImageButton( "##renderMode", m_spec.renderModeIcon, ImVec2{ 14.0f, 14.0f } ) )
    {
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "##stopButton", m_spec.stopIcon, ImVec2{ 14.0f, 14.0f } ) )
    {
      // kogayonon_physics::NvidiaPhysx::getInstance().switchState( false );
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "##startButton", m_spec.playIcon, ImVec2{ 14.0f, 14.0f } ) )
    {
      // kogayonon_physics::NvidiaPhysx::getInstance().switchState( true );
    }

    ImGui::PopStyleColor();
    ImGui::EndChild();
  }

  ImGui::PopStyleColor();
  ImGui::PopStyleVar( 2 );
}
