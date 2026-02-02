#include "utilities/configurator/configurator.hpp"
#include <fstream>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include "utilities/yaml_serializer/yaml_serializer.hpp"

namespace kogayonon_utilities
{

void Configurator::parseConfigFile()
{
  if ( !std::filesystem::exists( m_configPath ) )
  {
    spdlog::info( "Creating default config" );
    initDefaultConfig();
  }
  buildConfig();
}

void Configurator::writeConfig( const std::string& path )
{
  std::fstream ofs{ m_configPath.string(), std::ios::out };
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

  yamlSerializer->addKeyValuePair( "config", m_config );
}

void Configurator::buildConfig()
{
  auto doc = YAML::LoadFile( m_configPath.string() );

  m_config = doc.as<Config>();

  spdlog::info( "Config file read...\n\tWidth {}\n\tHeight {}\n\tMaximized? {}\n",
                m_config.width,
                m_config.height,
                m_config.maximized );
  m_loaded = true;
}
} // namespace kogayonon_utilities