#include "utilities/configurator/configurator.hpp"
#include <fstream>
#include <spdlog/spdlog.h>
#include "utilities/yaml_serializer/yaml_serializer.hpp"

namespace kogayonon_utilities
{
void Configurator::parseConfigFile()
{
  if ( !std::filesystem::exists( m_configPath ) )
  {
    initialiseDefaultConfig();
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

void Configurator::initialiseDefaultConfig()
{
  auto yamlSerializer = std::make_unique<YamlSerializer>( m_configPath.string() );
  // clang-format off
  yamlSerializer->beginMap()
      .addKey( "config" )
          .beginMap()

              .addKey("filters")
              .beginMap()
                  .addKey("files")
                  .beginSeq().addValue(".bin").endSeq()

                  .addKey("folders")
                  .beginSeq().addValue("scenes").endSeq()
              .endMap()// filters map

      .addKey("window")
          .beginMap()
              .addKeyValuePair("width",1900)
              .addKeyValuePair("height",800)
              .addKeyValuePair("maximized",true)
          .endMap()// window map

      .endMap();//config map

  // clang-format on 
  yamlSerializer.release();

  YAML::Node doc = YAML::LoadFile(m_configPath.string());
  // read the config here, must expose the structs and vectors to yaml i think
}

void Configurator::buildConfig()
{
}
} // namespace kogayonon_utilities