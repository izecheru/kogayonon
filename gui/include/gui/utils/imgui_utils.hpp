#pragma once
#include "precompiled/pch.hpp"
#include <imgui.h>

namespace gui_utils
{

#define RenderDisabled( content )                                                                                      \
  ImGui::BeginDisabled();                                                                                              \
  content;                                                                                                             \
  ImGui::EndDisabled();

// LA - low alpha

#define COL_BLACK IM_COL32( 0, 0, 0, 255 ) // r g b a
#define COL_WHITE IM_COL32( 255, 255, 255, 255 )
#define COL_RED IM_COL32( 255, 0, 0, 255 )
#define COL_RED_LA IM_COL32( 255, 0, 0, 25 )
#define COL_GREEN IM_COL32( 0, 255, 0, 255 )
#define COL_GREEN_LA IM_COL32( 0, 255, 0, 25 )
#define COL_BLUE IM_COL32( 0, 0, 255, 255 )
#define COL_BLUE_LA IM_COL32( 0, 0, 255, 25 )

#define COL_YELLOW IM_COL32( 255, 255, 0, 255 )
#define COL_CYAN IM_COL32( 0, 255, 255, 255 )
#define COL_MAGENTA IM_COL32( 255, 0, 255, 255 )

#define COL_GRAY IM_COL32( 128, 128, 128, 255 )
#define COL_DARK_GRAY IM_COL32( 64, 64, 64, 255 )
#define COL_LIGHT_GRAY IM_COL32( 192, 192, 192, 255 )
#define COL_LIGHT_GRAY_LA IM_COL32( 192, 192, 192, 25 )

#define COL_ORANGE IM_COL32( 255, 165, 0, 255 )
#define COL_PURPLE IM_COL32( 128, 0, 128, 255 )
#define COL_PINK IM_COL32( 255, 192, 203, 255 )

#define COL_BROWN IM_COL32( 139, 69, 19, 255 )
#define COL_LIME IM_COL32( 50, 205, 50, 255 )
#define COL_SKY_BLUE IM_COL32( 135, 206, 235, 255 )

#define COL_NAVY IM_COL32( 0, 0, 128, 255 )
#define COL_TEAL IM_COL32( 0, 128, 128, 255 )
#define COL_OLIVE IM_COL32( 128, 128, 0, 255 )

#define COL_GOLD IM_COL32( 255, 215, 0, 255 )
#define COL_SILVER IM_COL32( 192, 192, 192, 255 )

#define COL_TRANSPARENT IM_COL32( 0, 0, 0, 0 )

inline void renderWithSizedFont( ImFont* font, float size, auto&& func )
{
  ImGui::PushFont( font, size );
  func();
  ImGui::PopFont();
}

inline void renderWithFont( ImFont* font, auto&& func )
{
  ImGui::PushFont( font );
  func();
  ImGui::PopFont();
}

/**
 * @brief Center the popup window on the whole screen
 * @param coord This variable should have input from the SDL_GetWindowSize, height and width
 * @param size Size of the popup surface
 * @param cond ImGuiCond_None as default
 */
inline void centerPopup( const ImVec2& coord, const ImVec2& size, const ImGuiCond& cond = ImGuiCond_None )
{
  ImGui::SetNextWindowPos( { coord.x / 2.0f, coord.y / 2.0f }, cond, { 0.5f, 0.5f } );
  ImGui::SetNextWindowSize( size );
}

inline auto truncateText( const std::string& text, uint32_t limit ) -> std::string
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
inline void moveTextToCenter( const std::string& text )
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
inline void moveTextToCenter( ImVec2 size, const std::string& text )
{
  ImGui::SetCursorPosX( ImGui::GetCursorPosX() + ( ( size.x - ImGui::CalcTextSize( text.c_str() ).x ) * 0.5f ) +
                        2.0f * ImGui::GetStyle().FramePadding.x );
}

} // namespace gui_utils
