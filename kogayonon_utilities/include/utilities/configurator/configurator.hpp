#pragma once
#include <filesystem>
#include <string>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

namespace kogayonon_utilities
{
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

class Configurator
{
public:
  /**
   * @brief Parses the json config file and builds the struct above
   */
  static void parseConfigFile();

  /**
   * @brief Writes the current document to the config file
   */
  static void writeConfig( const std::string& path );

  /**
   * @brief Getter for config
   * @return Reference to the current Config
   */
  static Config& getConfig();

  /**
   * @brief Writes a deafult config
   */
  static void initDefaultConfig();

private:
  /**
   * @brief Populates the Config struct value with values loaded from the loaded json document
   */
  static void buildConfig();

  Configurator() = delete;
  ~Configurator() = default;

  Configurator& operator=( Configurator& ) = delete;
  Configurator( Configurator& ) = delete;

  static inline fs::path m_configPath{ fs::absolute( "config.yaml" ) };
  static inline Config m_config{};

  static inline bool m_loaded{ false };
};
} // namespace kogayonon_utilities

namespace YAML
{

template <>
struct convert<kogayonon_utilities::Config>
{
  static Node encode( const kogayonon_utilities::Config& rhs )
  {
    Node node;
    for ( auto i = 0u; i < rhs.fileFilters.size(); i++ )
    {
      node["filters"]["files"].push_back( rhs.fileFilters.at( i ) );
    }
    for ( auto i = 0u; i < rhs.folderFilters.size(); i++ )
    {
      node["filters"]["folders"].push_back( rhs.folderFilters.at( i ) );
    }

    node["window"]["width"] = rhs.width;
    node["window"]["height"] = rhs.height;
    node["window"]["maximized"] = rhs.maximized;
    return node;
  }

  static bool decode( const Node& node, kogayonon_utilities::Config& rhs )
  {
    if ( !node.IsMap() )
      return false;

    auto config = node["config"];
    auto filters = node["config"]["filters"];

    for ( auto i = 0u; i < filters["files"].size(); i++ )
    {
      rhs.fileFilters.push_back( filters["files"][i].as<std::string>() );
    }

    for ( auto i = 0u; i < filters["folders"].size(); i++ )
    {
      rhs.folderFilters.push_back( filters["folders"][i].as<std::string>() );
    }

    auto window = config["window"];
    rhs.height = window["height"].as<int>();
    rhs.width = window["width"].as<int>();
    rhs.maximized = window["maximized"].as<bool>();
    return true;
  }
};

inline Emitter& operator<<( Emitter& out, const kogayonon_utilities::Config& rhs )
{
  out << convert<kogayonon_utilities::Config>::encode( rhs );
  return out;
}

} // namespace YAML
