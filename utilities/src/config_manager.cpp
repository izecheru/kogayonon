#include "utilities/config_manager/config_manager.hpp"
#include <yaml-cpp/yaml.h>
#include "utilities/utils/utils.hpp"
#include "utilities/yaml_serializer/yaml_serializer.hpp"

namespace utilities
{

void EditorConfigManager::initConfig()
{
    if ( !std::filesystem::exists( m_configPath ) )
    {
        KINFO( "Creating default config" );
        initDefaultConfig();
    }

    if ( !std::filesystem::exists( m_colorConfigPath ) )
    {
        KINFO( "Creating default color config" );
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
        .ImGuiCol_Text = { 0.92f, 0.92f, 0.92f, 1.00f },
        .ImGuiCol_ModalWindowDimBg = { 0.00f, 0.00f, 0.00f, 0.45f },
        .ImGuiCol_TextDisabled = { 0.48f, 0.48f, 0.48f, 1.00f },
        .ImGuiCol_WindowBg = { 0.12f, 0.13f, 0.15f, 0.98f },
        .ImGuiCol_ChildBg = { 0.15f, 0.16f, 0.18f, 0.75f },
        .ImGuiCol_PopupBg = { 0.10f, 0.11f, 0.13f, 1.00f },
        .ImGuiCol_Border = { 0.38f, 0.40f, 0.44f, 0.85f },
        .ImGuiCol_BorderShadow = { 0.00f, 0.00f, 0.00f, 0.00f },
        .ImGuiCol_TabHovered = { 0.25f, 0.45f, 0.70f, 1.00f },
        .ImGuiCol_TabActive = { 0.72f, 0.58f, 0.32f, 1.00f },
        .ImGuiCol_TabSelected = { 0.72f, 0.58f, 0.32f, 1.00f },
        .ImGuiCol_TabSelectedOverline = { 0.90f, 0.72f, 0.40f, 1.00f },
        .ImGuiCol_TabDimmed = { 0.18f, 0.19f, 0.21f, 1.00f },
        .ImGuiCol_TabDimmedSelected = { 0.45f, 0.36f, 0.22f, 1.00f },
        .ImGuiCol_FrameBg = { 0.18f, 0.19f, 0.21f, 1.00f },
        .ImGuiCol_FrameBgHovered = { 0.28f, 0.42f, 0.62f, 1.00f },
        .ImGuiCol_FrameBgActive = { 0.34f, 0.50f, 0.72f, 1.00f },
        .ImGuiCol_TitleBg = { 0.08f, 0.08f, 0.10f, 1.00f },
        .ImGuiCol_TitleBgCollapsed = { 0.10f, 0.14f, 0.20f, 1.00f },
        .ImGuiCol_TitleBgActive = { 0.18f, 0.24f, 0.34f, 1.00f },
        .ImGuiCol_MenuBarBg = { 0.10f, 0.11f, 0.13f, 1.00f },
        .ImGuiCol_ScrollbarBg = { 0.10f, 0.11f, 0.13f, 0.85f },
        .ImGuiCol_ScrollbarGrab = { 0.32f, 0.36f, 0.42f, 0.90f },
        .ImGuiCol_ScrollbarGrabHovered = { 0.42f, 0.48f, 0.56f, 1.00f },
        .ImGuiCol_ScrollbarGrabActive = { 0.52f, 0.58f, 0.68f, 1.00f },
        .ImGuiCol_CheckMark = { 0.95f, 0.95f, 0.95f, 1.00f },
        .ImGuiCol_SliderGrab = { 0.70f, 0.74f, 0.80f, 1.00f },
        .ImGuiCol_SliderGrabActive = { 0.90f, 0.80f, 0.50f, 1.00f },
        .ImGuiCol_Button = { 0.22f, 0.24f, 0.28f, 1.00f },
        .ImGuiCol_ButtonHovered = { 0.30f, 0.44f, 0.64f, 1.00f },
        .ImGuiCol_ButtonActive = { 0.38f, 0.54f, 0.78f, 1.00f },
        .ImGuiCol_Header = { 0.22f, 0.24f, 0.28f, 1.00f },
        .ImGuiCol_HeaderHovered = { 0.30f, 0.44f, 0.64f, 1.00f },
        .ImGuiCol_HeaderActive = { 0.38f, 0.54f, 0.78f, 1.00f },
        .ImGuiCol_Separator = { 0.42f, 0.44f, 0.48f, 1.00f },
        .ImGuiCol_SeparatorHovered = { 0.55f, 0.60f, 0.68f, 1.00f },
        .ImGuiCol_SeparatorActive = { 0.70f, 0.76f, 0.86f, 1.00f },
        .ImGuiCol_ResizeGrip = { 0.60f, 0.64f, 0.70f, 0.45f },
        .ImGuiCol_ResizeGripHovered = { 0.75f, 0.80f, 0.88f, 0.75f },
        .ImGuiCol_ResizeGripActive = { 0.90f, 0.94f, 1.00f, 0.95f },
        .ImGuiCol_PlotLines = { 0.70f, 0.70f, 0.70f, 1.00f },
        .ImGuiCol_PlotLinesHovered = { 1.00f, 0.55f, 0.35f, 1.00f },
        .ImGuiCol_PlotHistogram = { 0.90f, 0.70f, 0.20f, 1.00f },
        .ImGuiCol_PlotHistogramHovered = { 1.00f, 0.80f, 0.30f, 1.00f },
        .ImGuiCol_TextSelectedBg = { 0.26f, 0.48f, 0.82f, 0.85f },
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

    KINFO( "config loaded" );
    m_loaded = true;
}
} // namespace utilities