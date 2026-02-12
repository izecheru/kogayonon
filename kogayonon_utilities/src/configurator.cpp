#include "utilities/configurator/configurator.hpp"
#include <fstream>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include "utilities/yaml_serializer/yaml_serializer.hpp"

namespace kogayonon_utilities
{

void Configurator::initConfig()
{
  if ( !std::filesystem::exists( m_configPath ) )
  {
    spdlog::info( "Creating default config" );
    initDefaultConfig();
  }
  parseConfig();
}

void Configurator::writeConfig()
{
  auto yamlSerializer = std::make_unique<YamlSerializer>( m_configPath.string() );
  yamlSerializer->addValue( m_config );
}

auto Configurator::getConfig() -> Config&
{
  assert( m_loaded && "document was not loaded correctly" );
  return m_config;
}

void Configurator::initDefaultConfig()
{
  auto yamlSerializer = std::make_unique<YamlSerializer>( m_configPath.string() );

  m_config = Config{ .width = 1900,
                     .height = 800,
                     .maximized = true,

                     .fileFilters = { ".bin" },
                     .folderFilters = { "scenes", "fonts" } };

  yamlSerializer->addValue( m_config );
}

void Configurator::parseConfig()
{
  auto doc = YAML::LoadFile( m_configPath.string() );

  // since only config lives in here the whole node is config
  m_config = doc.as<Config>();

  spdlog::info( "config loaded" );
  m_loaded = true;
}
} // namespace kogayonon_utilities