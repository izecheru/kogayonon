#pragma once
#include <imgui.h>
#include <string>

namespace gui_utils
{

#define RenderDisabled( content )                                                                                      \
  ImGui::BeginDisabled();                                                                                              \
  content ImGui::EndDisabled();

#define RenderWithSizedFont( font, size, func )                                                                        \
  ImGui::PushFont( font, size );                                                                                       \
  func;                                                                                                                \
  ImGui::PopFont();

#define RenderWithFont( font, func )                                                                                   \
  ImGui::PushFont( font );                                                                                             \
  func;                                                                                                                \
  ImGui::PopFont();

static auto truncateText( const std::string& text, uint32_t limit ) -> std::string
{
  auto trucatedText = text;

  const float textWidth = ImGui::CalcTextSize( text.c_str(), nullptr, true ).x;

  if ( textWidth < limit )
    return text;

  constexpr const char* ELLIPSIS = " ...";
  const float ellipsisSize = ImGui::CalcTextSize( ELLIPSIS ).x;

  int visibleCharacters = 0;
  for ( auto i = 0u; i < text.size(); i++ )
  {
    const float currentWidth = ImGui::CalcTextSize( text.substr( 0, i ).c_str(), nullptr, true ).x;
    if ( currentWidth + ellipsisSize > limit )
    {
      break;
    }

    visibleCharacters = i;
  }

  trucatedText = ( text.substr( 0, visibleCharacters ) + ELLIPSIS ).c_str();

  return trucatedText;
}

static void moveTextToCenter( const std::string& text )
{
  ImGuiStyle& style = ImGui::GetStyle();
  float size = ImGui::CalcTextSize( text.c_str() ).x + ( style.FramePadding.x * 2.0f );
  float avail = ImGui::GetContentRegionAvail().x;

  float off = ( avail - size ) * 0.5f;

  if ( off > 0.0f )
    ImGui::SetCursorPosX( ImGui::GetCursorPosX() + off );
}
} // namespace gui_utils
