#pragma once
#include <imgui.h>
#include <string>

namespace ImGui_Utils
{
/**
 * @brief Truncate the text if the strlen(text) > is bigger than the truncateWidth
 * @param text Text we will truncate
 * @param truncateWidth The limit at which we place the ellipsis if the text len surpasses the limit
 * @return String representing the resulted text after truncation, or just the text if it does not go above the limit
 * imposed
 */
static std::string truncateText( const std::string& text, float truncateWidth )
{
  std::string truncatedText = text;

  const float textWidth = ImGui::CalcTextSize( text.c_str(), nullptr, true ).x;

  if ( textWidth > truncateWidth )
  {
    constexpr const char* ELLIPSIS = "...";
    const float ellipsisSize = ImGui::CalcTextSize( ELLIPSIS ).x;

    int visibleChars = 0;
    for ( size_t i = 0; i < text.size(); i++ )
    {
      const float currWidth = ImGui::CalcTextSize( text.substr( 0, i ).c_str(), nullptr, true ).x;
      if ( currWidth + ellipsisSize > truncateWidth )
      {
        break;
      }

      visibleChars = i;
    }

    truncatedText = text.substr( 0, visibleChars ) + ELLIPSIS;
  }

  return truncatedText;
}

static bool isFocusedAndHovered( bool focused, bool hovered )
{
  return focused && hovered;
}
} // namespace ImGui_Utils