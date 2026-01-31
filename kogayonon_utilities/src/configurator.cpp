#include "utilities/configurator/configurator.hpp"
#include <fstream>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include "utilities/yaml_serializer/yaml_serializer.hpp"

namespace YAML
{
template <>
struct convert<kogayonon_utilities::Config>
{
  static Node encode( const kogayonon_utilities::Config& rhs )
  {
    Node node;
    return node;
  }

  static bool decode( const Node& node, kogayonon_utilities::Config& rhs )
  {
    if ( !node.IsSequence() || node.size() != 3 )
      return false;
    return true;
  }
};
} // namespace YAML

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
}

void Configurator::buildConfig()
{
  auto doc = YAML::LoadFile(m_configPath.string());

  m_config.height = doc["config"]["window"]["height"].as<int>();
  m_config.width= doc["config"]["window"]["width"].as<int>();
  m_config.maximized= doc["config"]["window"]["maximized"].as<bool>();

  auto files = doc["config"]["filters"]["files"];
  auto folders = doc["config"]["filters"]["folders"];

  for(uint32_t i = 0u;i<files.size();i++)
  {
    m_config.fileFilters.push_back(files[i].as<std::string>());
  }

  for(uint32_t i = 0u;i<folders.size();i++)
  {
    m_config.folderFilters.push_back(folders[i].as<std::string>());
  }

  spdlog::info("Config file read...\n\tWidth {}\n\tHeight {}\n\tMaximized? {}\n",m_config.width,m_config.height, m_config.maximized);
  m_loaded=true;
}
} // namespace kogayonon_utilities