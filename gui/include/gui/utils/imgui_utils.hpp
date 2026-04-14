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

  constexpr const char* ELLIPSIS = "...";
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

/**
 * @brief Moves the cursor to the center of the available space based on text length and frame padding
 * @param text
 */
static void moveTextToCenter( const std::string& text )
{
  ImGui::SetCursorPosX( ImGui::GetCursorPosX() +
                        ( ( ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize( text.c_str() ).x ) * 0.5f ) +
                        2.0f * ImGui::GetStyle().FramePadding.x );
}

/**
 * @brief Moves the cursor to the center of the provided space based on text length and frame padding
 * @param size
 * @param text
 */
static void moveTextToCenter( ImVec2 size, const std::string& text )
{
  ImGui::SetCursorPosX( ImGui::GetCursorPosX() + ( ( size.x - ImGui::CalcTextSize( text.c_str() ).x ) * 0.5f ) +
                        2.0f * ImGui::GetStyle().FramePadding.x );
}

} // namespace gui_utils
