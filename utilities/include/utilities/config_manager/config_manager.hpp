#pragma once
#include <yaml-cpp/yaml.h>
#include "precompiled/pch.hpp"

namespace fs = std::filesystem;

namespace utilities
{

struct ColorVec4
{
  float r{ 0.0f };
  float g{ 0.0f };
  float b{ 0.0f };
  float alpha{ 0.0f };
};

struct ColorConfig
{
  ColorVec4 ImGuiCol_Text;
  ColorVec4 ImGuiCol_ModalWindowDimBg;
  ColorVec4 ImGuiCol_TextDisabled;
  ColorVec4 ImGuiCol_WindowBg;
  ColorVec4 ImGuiCol_ChildBg;
  ColorVec4 ImGuiCol_PopupBg;
  ColorVec4 ImGuiCol_Border;
  ColorVec4 ImGuiCol_BorderShadow;
  ColorVec4 ImGuiCol_TabHovered;
  ColorVec4 ImGuiCol_TabActive;
  ColorVec4 ImGuiCol_TabSelected;
  ColorVec4 ImGuiCol_TabSelectedOverline;
  ColorVec4 ImGuiCol_TabDimmed;
  ColorVec4 ImGuiCol_TabDimmedSelected;
  ColorVec4 ImGuiCol_FrameBg;
  ColorVec4 ImGuiCol_FrameBgHovered;
  ColorVec4 ImGuiCol_FrameBgActive;
  ColorVec4 ImGuiCol_TitleBg;
  ColorVec4 ImGuiCol_TitleBgCollapsed;
  ColorVec4 ImGuiCol_TitleBgActive;
  ColorVec4 ImGuiCol_MenuBarBg;
  ColorVec4 ImGuiCol_ScrollbarBg;
  ColorVec4 ImGuiCol_ScrollbarGrab;
  ColorVec4 ImGuiCol_ScrollbarGrabHovered;
  ColorVec4 ImGuiCol_ScrollbarGrabActive;
  ColorVec4 ImGuiCol_CheckMark;
  ColorVec4 ImGuiCol_SliderGrab;
  ColorVec4 ImGuiCol_SliderGrabActive;
  ColorVec4 ImGuiCol_Button;
  ColorVec4 ImGuiCol_ButtonHovered;
  ColorVec4 ImGuiCol_ButtonActive;
  ColorVec4 ImGuiCol_Header;
  ColorVec4 ImGuiCol_HeaderHovered;
  ColorVec4 ImGuiCol_HeaderActive;
  ColorVec4 ImGuiCol_Separator;
  ColorVec4 ImGuiCol_SeparatorHovered;
  ColorVec4 ImGuiCol_SeparatorActive;
  ColorVec4 ImGuiCol_ResizeGrip;
  ColorVec4 ImGuiCol_ResizeGripHovered;
  ColorVec4 ImGuiCol_ResizeGripActive;
  ColorVec4 ImGuiCol_PlotLines;
  ColorVec4 ImGuiCol_PlotLinesHovered;
  ColorVec4 ImGuiCol_PlotHistogram;
  ColorVec4 ImGuiCol_PlotHistogramHovered;
  ColorVec4 ImGuiCol_TextSelectedBg;
};

struct Config
{
  // window
  int width{ 0 };
  int height{ 0 };
  bool maximized{ false };

  // filters
  std::vector<std::string> fileFilters;
  std::vector<std::string> folderFilters;
};

class EditorConfigManager
{
public:
  static void initConfig();

  /**
   * @brief Writes the current document to the config file
   */
  static void writeConfig();
  static void writeColorConfig();

  /**
   * @brief Getter for config
   * @return Reference to the current Config
   */
  static auto getConfig() -> Config&;
  static auto getColorConfig() -> ColorConfig&;

  /**
   * @brief Writes a deafult config
   */
  static void initDefaultConfig();
  static void initDefaultColorConfig();

private:
  /**
   * @brief Populates the Config struct value with values loaded from the loaded json document
   */
  static void parseConfig();

  EditorConfigManager() = delete;
  ~EditorConfigManager() = default;

  EditorConfigManager& operator=( EditorConfigManager& ) = delete;
  EditorConfigManager( EditorConfigManager& ) = delete;

  static inline fs::path m_configPath{ fs::absolute( "config.yaml" ) };
  static inline Config m_config{};

  static inline fs::path m_colorConfigPath{ fs::absolute( "colorConfig.yaml" ) };
  static inline ColorConfig m_colorConfig{};

  static inline bool m_loaded{ false };
};
} // namespace utilities

namespace YAML
{

template <>
struct convert<utilities::ColorVec4>
{
  static Node encode( const utilities::ColorVec4& v )
  {
    Node node;
    node["r"] = v.r;
    node["g"] = v.g;
    node["b"] = v.b;
    node["alpha"] = v.alpha;
    return node;
  }

  static bool decode( const Node& node, utilities::ColorVec4& v )
  {
    if ( !node.IsMap() )
      return false;
    v.r = node["r"].as<float>();
    v.g = node["g"].as<float>();
    v.b = node["b"].as<float>();
    v.alpha = node["alpha"].as<float>();
    return true;
  }
};

template <>
struct convert<utilities::ColorConfig>
{
  static Node encode( const utilities::ColorConfig& rhs )
  {
    Node node;
    Node cfg;

    cfg["ImGuiCol_Text"] = rhs.ImGuiCol_Text;
    cfg["ImGuiCol_ModalWindowDimBg"] = rhs.ImGuiCol_ModalWindowDimBg;
    cfg["ImGuiCol_TextDisabled"] = rhs.ImGuiCol_TextDisabled;
    cfg["ImGuiCol_WindowBg"] = rhs.ImGuiCol_WindowBg;
    cfg["ImGuiCol_ChildBg"] = rhs.ImGuiCol_ChildBg;
    cfg["ImGuiCol_PopupBg"] = rhs.ImGuiCol_PopupBg;
    cfg["ImGuiCol_Border"] = rhs.ImGuiCol_Border;
    cfg["ImGuiCol_BorderShadow"] = rhs.ImGuiCol_BorderShadow;
    cfg["ImGuiCol_TabHovered"] = rhs.ImGuiCol_TabHovered;
    cfg["ImGuiCol_TabActive"] = rhs.ImGuiCol_TabActive;
    cfg["ImGuiCol_TabSelected"] = rhs.ImGuiCol_TabSelected;
    cfg["ImGuiCol_TabSelectedOverline"] = rhs.ImGuiCol_TabSelectedOverline;
    cfg["ImGuiCol_TabDimmed"] = rhs.ImGuiCol_TabDimmed;
    cfg["ImGuiCol_TabDimmedSelected"] = rhs.ImGuiCol_TabDimmedSelected;
    cfg["ImGuiCol_FrameBg"] = rhs.ImGuiCol_FrameBg;
    cfg["ImGuiCol_FrameBgHovered"] = rhs.ImGuiCol_FrameBgHovered;
    cfg["ImGuiCol_FrameBgActive"] = rhs.ImGuiCol_FrameBgActive;
    cfg["ImGuiCol_TitleBg"] = rhs.ImGuiCol_TitleBg;
    cfg["ImGuiCol_TitleBgCollapsed"] = rhs.ImGuiCol_TitleBgCollapsed;
    cfg["ImGuiCol_TitleBgActive"] = rhs.ImGuiCol_TitleBgActive;
    cfg["ImGuiCol_MenuBarBg"] = rhs.ImGuiCol_MenuBarBg;
    cfg["ImGuiCol_ScrollbarBg"] = rhs.ImGuiCol_ScrollbarBg;
    cfg["ImGuiCol_ScrollbarGrab"] = rhs.ImGuiCol_ScrollbarGrab;
    cfg["ImGuiCol_ScrollbarGrabHovered"] = rhs.ImGuiCol_ScrollbarGrabHovered;
    cfg["ImGuiCol_ScrollbarGrabActive"] = rhs.ImGuiCol_ScrollbarGrabActive;
    cfg["ImGuiCol_CheckMark"] = rhs.ImGuiCol_CheckMark;
    cfg["ImGuiCol_SliderGrab"] = rhs.ImGuiCol_SliderGrab;
    cfg["ImGuiCol_SliderGrabActive"] = rhs.ImGuiCol_SliderGrabActive;
    cfg["ImGuiCol_Button"] = rhs.ImGuiCol_Button;
    cfg["ImGuiCol_ButtonHovered"] = rhs.ImGuiCol_ButtonHovered;
    cfg["ImGuiCol_ButtonActive"] = rhs.ImGuiCol_ButtonActive;
    cfg["ImGuiCol_Header"] = rhs.ImGuiCol_Header;
    cfg["ImGuiCol_HeaderHovered"] = rhs.ImGuiCol_HeaderHovered;
    cfg["ImGuiCol_HeaderActive"] = rhs.ImGuiCol_HeaderActive;
    cfg["ImGuiCol_Separator"] = rhs.ImGuiCol_Separator;
    cfg["ImGuiCol_SeparatorHovered"] = rhs.ImGuiCol_SeparatorHovered;
    cfg["ImGuiCol_SeparatorActive"] = rhs.ImGuiCol_SeparatorActive;
    cfg["ImGuiCol_ResizeGrip"] = rhs.ImGuiCol_ResizeGrip;
    cfg["ImGuiCol_ResizeGripHovered"] = rhs.ImGuiCol_ResizeGripHovered;
    cfg["ImGuiCol_ResizeGripActive"] = rhs.ImGuiCol_ResizeGripActive;
    cfg["ImGuiCol_PlotLines"] = rhs.ImGuiCol_PlotLines;
    cfg["ImGuiCol_PlotLinesHovered"] = rhs.ImGuiCol_PlotLinesHovered;
    cfg["ImGuiCol_PlotHistogram"] = rhs.ImGuiCol_PlotHistogram;
    cfg["ImGuiCol_PlotHistogramHovered"] = rhs.ImGuiCol_PlotHistogramHovered;
    cfg["ImGuiCol_TextSelectedBg"] = rhs.ImGuiCol_TextSelectedBg;

    node["color_config"] = cfg;
    return node;
  }

  static bool decode( const Node& node, utilities::ColorConfig& rhs )
  {
    if ( !node["color_config"] )
      return false;
    const auto& cfg = node["color_config"];

    rhs.ImGuiCol_Text = cfg["ImGuiCol_Text"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ModalWindowDimBg = cfg["ImGuiCol_ModalWindowDimBg"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TextDisabled = cfg["ImGuiCol_TextDisabled"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_WindowBg = cfg["ImGuiCol_WindowBg"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ChildBg = cfg["ImGuiCol_ChildBg"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_PopupBg = cfg["ImGuiCol_PopupBg"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_Border = cfg["ImGuiCol_Border"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_BorderShadow = cfg["ImGuiCol_BorderShadow"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TabHovered = cfg["ImGuiCol_TabHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TabActive = cfg["ImGuiCol_TabActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TabSelected = cfg["ImGuiCol_TabSelected"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TabSelectedOverline = cfg["ImGuiCol_TabSelectedOverline"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TabDimmed = cfg["ImGuiCol_TabDimmed"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TabDimmedSelected = cfg["ImGuiCol_TabDimmedSelected"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_FrameBg = cfg["ImGuiCol_FrameBg"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_FrameBgHovered = cfg["ImGuiCol_FrameBgHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_FrameBgActive = cfg["ImGuiCol_FrameBgActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TitleBg = cfg["ImGuiCol_TitleBg"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TitleBgCollapsed = cfg["ImGuiCol_TitleBgCollapsed"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TitleBgActive = cfg["ImGuiCol_TitleBgActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_MenuBarBg = cfg["ImGuiCol_MenuBarBg"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ScrollbarBg = cfg["ImGuiCol_ScrollbarBg"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ScrollbarGrab = cfg["ImGuiCol_ScrollbarGrab"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ScrollbarGrabHovered = cfg["ImGuiCol_ScrollbarGrabHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ScrollbarGrabActive = cfg["ImGuiCol_ScrollbarGrabActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_CheckMark = cfg["ImGuiCol_CheckMark"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_SliderGrab = cfg["ImGuiCol_SliderGrab"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_SliderGrabActive = cfg["ImGuiCol_SliderGrabActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_Button = cfg["ImGuiCol_Button"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ButtonHovered = cfg["ImGuiCol_ButtonHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ButtonActive = cfg["ImGuiCol_ButtonActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_Header = cfg["ImGuiCol_Header"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_HeaderHovered = cfg["ImGuiCol_HeaderHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_HeaderActive = cfg["ImGuiCol_HeaderActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_Separator = cfg["ImGuiCol_Separator"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_SeparatorHovered = cfg["ImGuiCol_SeparatorHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_SeparatorActive = cfg["ImGuiCol_SeparatorActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ResizeGrip = cfg["ImGuiCol_ResizeGrip"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ResizeGripHovered = cfg["ImGuiCol_ResizeGripHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_ResizeGripActive = cfg["ImGuiCol_ResizeGripActive"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_PlotLines = cfg["ImGuiCol_PlotLines"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_PlotLinesHovered = cfg["ImGuiCol_PlotLinesHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_PlotHistogram = cfg["ImGuiCol_PlotHistogram"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_PlotHistogramHovered = cfg["ImGuiCol_PlotHistogramHovered"].as<utilities::ColorVec4>();
    rhs.ImGuiCol_TextSelectedBg = cfg["ImGuiCol_TextSelectedBg"].as<utilities::ColorVec4>();

    return true;
  }
};

inline Emitter& operator<<( Emitter& out, const utilities::ColorConfig& rhs )
{
  out << convert<utilities::ColorConfig>::encode( rhs );
  return out;
}

template <>
struct convert<utilities::Config>
{
  static Node encode( const utilities::Config& rhs )
  {
    Node node;
    auto config = node["config"];
    config["window"]["width"] = rhs.width;
    config["window"]["height"] = rhs.height;
    config["window"]["maximized"] = rhs.maximized;
    config["filters"]["files"] = rhs.fileFilters;
    config["filters"]["folders"] = rhs.folderFilters;
    return node;
  }

  static bool decode( const Node& node, utilities::Config& rhs )
  {
    if ( !node["config"] )
      return false;

    auto& config = node["config"];
    rhs.width = config["window"]["width"].as<int>();
    rhs.height = config["window"]["height"].as<int>();
    rhs.maximized = config["window"]["maximized"].as<bool>();
    rhs.fileFilters = config["filters"]["files"].as<std::vector<std::string>>();
    rhs.folderFilters = config["filters"]["folders"].as<std::vector<std::string>>();
    return true;
  }
};

inline Emitter& operator<<( Emitter& out, const utilities::Config& rhs )
{
  out << convert<utilities::Config>::encode( rhs );
  return out;
}

} // namespace YAML
