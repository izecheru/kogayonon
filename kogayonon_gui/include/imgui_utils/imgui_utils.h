#pragma once
#include <assert.h>
#include <imgui.h>
#include <string>

namespace ImGui_Utils
{
template <typename Func>
auto addPaddedGui( Func&& func, ImVec2 padding )
{
  auto cursor = ImGui::GetCursorPos();
  ImGui::SetCursorPos( { cursor.x + padding.x, cursor.y + padding.y } );
  func();

  // this needs to be here cause imgui will hit an assert
  if ( padding.x > 0 || padding.y > 0 )
    ImGui::Dummy( padding );
}

/**
 * @brief Truncate text and add ellipsis if the limit is surpassed
 * @param text
 * @param limit
 * @return The truncated text with ellipsis at the end
 * https://github.com/ocornut/imgui/issues/5267#issuecomment-2408993109
 */
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

/**
 * @brief Structure for window padding, since this is created on the stack it will get destroyed after ImGui::End() so
 * the next window won't get the padding values
 */
struct ScopedPadding
{
  explicit ScopedPadding( ImVec2 padding )
  {
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, padding );
  }

  ~ScopedPadding()
  {
    ImGui::PopStyleVar();
  }
};

static void addPadding( ImVec2 padding = ImVec2{ 10.0f, 10.0f } )
{
  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, padding );
}

static void removePadding()
{
  ImGui::PopStyleVar();
}

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