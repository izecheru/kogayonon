#include "utilities/config_manager/config_manager.hpp"
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include "utilities/yaml_serializer/yaml_serializer.hpp"

namespace utilities
{

void EditorConfigManager::initConfig()
{
  if ( !std::filesystem::exists( m_configPath ) )
  {
    spdlog::info( "Creating default config" );
    initDefaultConfig();
  }

  if ( !std::filesystem::exists( m_colorConfigPath ) )
  {
    spdlog::info( "Creating default color config" );
    initDefaultColorConfig();
  }

  parseConfig();
}

void EditorConfigManager::writeConfig()
{
  auto yamlSerializer = std::make_unique<YamlSerializer>( m_configPath.string() );
  yamlSerializer->addValue( m_config );
}

void EditorConfigManager::writeColorConfig()
{
  auto yamlSerializer = std::make_unique<YamlSerializer>( m_colorConfigPath.string() );
  yamlSerializer->addValue( m_colorConfig );
}

auto EditorConfigManager::getConfig() -> Config&
{
  assert( m_loaded && "document was not loaded correctly" );
  return m_config;
}

auto EditorConfigManager::getColorConfig() -> ColorConfig&
{
  return m_colorConfig;
}

void EditorConfigManager::initDefaultConfig()
{
  auto yamlSerializer = std::make_unique<YamlSerializer>( m_configPath.string() );

  m_config = Config{ .width = 1900,
                     .height = 800,
                     .maximized = true,
                     .fileFilters = { ".bin" },
                     .folderFilters = { "scenes", "fonts" } };

  yamlSerializer->addValue( m_config );
}

void EditorConfigManager::initDefaultColorConfig()
{
  auto yamlSerializer = std::make_unique<YamlSerializer>( m_colorConfigPath.string() );

  m_colorConfig = ColorConfig{
    .ImGuiCol_Text = { 0.73333335f, 0.73333335f, 0.73333335f, 1.00f },
    .ImGuiCol_ModalWindowDimBg = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ImGuiCol_TextDisabled = { 0.34509805f, 0.34509805f, 0.34509805f, 1.00f },
    .ImGuiCol_WindowBg = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f },
    .ImGuiCol_ChildBg = { 0.23529413f, 0.24705884f, 0.25490198f, 0.00f },
    .ImGuiCol_PopupBg = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f },
    .ImGuiCol_Border = { 0.33333334f, 0.33333334f, 0.33333334f, 0.50f },
    .ImGuiCol_BorderShadow = { 0.15686275f, 0.15686275f, 0.15686275f, 0.00f },
    .ImGuiCol_TabHovered = { 0.183f, 0.345f, 0.443f, 1.0f },
    .ImGuiCol_TabActive = { 0.658f, 0.531f, 0.316f, 1.0f },
    .ImGuiCol_TabSelected = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ImGuiCol_TabSelectedOverline = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ImGuiCol_TabDimmed = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ImGuiCol_TabDimmedSelected = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ImGuiCol_FrameBg = { 0.16862746f, 0.16862746f, 0.16862746f, 0.54f },
    .ImGuiCol_FrameBgHovered = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f },
    .ImGuiCol_FrameBgActive = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f },
    .ImGuiCol_TitleBg = { 0.04f, 0.04f, 0.04f, 1.00f },
    .ImGuiCol_TitleBgCollapsed = { 0.16f, 0.29f, 0.48f, 1.00f },
    .ImGuiCol_TitleBgActive = { 0.741f, 0.576f, 0.976f, 0.094f },
    .ImGuiCol_MenuBarBg = { 0.27058825f, 0.28627452f, 0.2901961f, 0.80f },
    .ImGuiCol_ScrollbarBg = { 0.27058825f, 0.28627452f, 0.2901961f, 0.60f },
    .ImGuiCol_ScrollbarGrab = { 0.21960786f, 0.30980393f, 0.41960788f, 0.51f },
    .ImGuiCol_ScrollbarGrabHovered = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f },
    .ImGuiCol_ScrollbarGrabActive = { 0.13725491f, 0.19215688f, 0.2627451f, 0.91f },
    .ImGuiCol_CheckMark = { 0.90f, 0.90f, 0.90f, 0.83f },
    .ImGuiCol_SliderGrab = { 0.70f, 0.70f, 0.70f, 0.62f },
    .ImGuiCol_SliderGrabActive = { 0.30f, 0.30f, 0.30f, 0.84f },
    .ImGuiCol_Button = { 0.33333334f, 0.3529412f, 0.36078432f, 0.49f },
    .ImGuiCol_ButtonHovered = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f },
    .ImGuiCol_ButtonActive = { 0.13725491f, 0.19215688f, 0.2627451f, 1.00f },
    .ImGuiCol_Header = { 0.33333334f, 0.3529412f, 0.36078432f, 0.53f },
    .ImGuiCol_HeaderHovered = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f },
    .ImGuiCol_HeaderActive = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f },
    .ImGuiCol_Separator = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f },
    .ImGuiCol_SeparatorHovered = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f },
    .ImGuiCol_SeparatorActive = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f },
    .ImGuiCol_ResizeGrip = { 1.00f, 1.00f, 1.00f, 0.85f },
    .ImGuiCol_ResizeGripHovered = { 1.00f, 1.00f, 1.00f, 0.60f },
    .ImGuiCol_ResizeGripActive = { 1.00f, 1.00f, 1.00f, 0.90f },
    .ImGuiCol_PlotLines = { 0.61f, 0.61f, 0.61f, 1.00f },
    .ImGuiCol_PlotLinesHovered = { 1.00f, 0.43f, 0.35f, 1.00f },
    .ImGuiCol_PlotHistogram = { 0.90f, 0.70f, 0.00f, 1.00f },
    .ImGuiCol_PlotHistogramHovered = { 1.00f, 0.60f, 0.00f, 1.00f },
    .ImGuiCol_TextSelectedBg = { 0.18431373f, 0.39607847f, 0.79215693f, 0.90f },
  };

  yamlSerializer->addValue( m_colorConfig );
}

void EditorConfigManager::parseConfig()
{
  auto doc = YAML::LoadFile( m_configPath.string() );
  auto colorDoc = YAML::LoadFile( m_colorConfigPath.string() );

  // since only config lives in here the whole node is config
  m_config = doc.as<Config>();
  m_colorConfig = colorDoc.as<ColorConfig>();

  spdlog::info( "config loaded" );
  m_loaded = true;
}
} // namespace utilities