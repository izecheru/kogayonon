#pragma once
#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <string>

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
  static void writeConfig();

  /**
   * @brief Gets a reference to the current document
   * @return rapidjson::Document reference
   */
  static rapidjson::Document& getDocument();

  /**
   * @brief Getter for config
   * @return Reference to the current Config
   */
  static Config& getConfig();

  /**
   * @brief Writes a deafult config
   */
  static void initialiseDefaultConfig();

private:
  /**
   * @brief Populates the Config struct value with values loaded from the loaded json document
   */
  static void buildConfig();

  Configurator() = delete;
  ~Configurator() = default;

  Configurator& operator=( Configurator& ) = delete;
  Configurator( Configurator& ) = delete;

  static inline std::filesystem::path m_configPath{ std::filesystem::absolute( "config.json" ) };
  static inline rapidjson::Document m_jsonDocument;
  static inline Config m_config{};

  static inline bool m_loaded{ false };
};
} // namespace kogayonon_utilities