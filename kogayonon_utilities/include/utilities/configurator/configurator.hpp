#pragma once
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace kogayonon_utilities
{
struct Config
{
  // window
  int width;
  int height;
  bool maximized;

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