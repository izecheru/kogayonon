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
  static void initConfig();

  /**
   * @brief Writes the current document to the config file
   */
  static void writeConfig();

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
  static void parseConfig();

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
    auto config = node["config"];
    config["window"]["width"] = rhs.width;
    config["window"]["height"] = rhs.height;
    config["window"]["maximized"] = rhs.maximized;
    config["filters"]["files"] = rhs.fileFilters;
    config["filters"]["folders"] = rhs.folderFilters;
    return node;
  }

  static bool decode( const Node& node, kogayonon_utilities::Config& rhs )
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

inline Emitter& operator<<( Emitter& out, const kogayonon_utilities::Config& rhs )
{
  out << convert<kogayonon_utilities::Config>::encode( rhs );
  return out;
}

} // namespace YAML
