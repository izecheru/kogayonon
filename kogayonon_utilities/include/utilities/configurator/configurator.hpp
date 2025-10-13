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

  /**
   * @brief Check if file has this object and if it is valid
   * @param toCheck Value to check for
   * @param member Member name
   * @return True if member exists and is of type Object
   */
  static bool checkObject( const rapidjson::Value& toCheck, const std::string& member );

  /**
   * @brief Check if file has this array and if it is valid
   * @param toCheck Array to check for
   * @param member Member name
   * @return True if member exists and is of type Array
   */
  static bool checkArray( const rapidjson::Value& toCheck, const std::string& member );

  Configurator() = default;
  ~Configurator() = default;

  Configurator& operator=( Configurator& ) = delete;
  Configurator( Configurator& ) = delete;

  static inline std::filesystem::path m_configPath{ std::filesystem::absolute( "config.json" ) };
  static inline rapidjson::Document m_jsonDocument;
  static inline Config m_config{};

  static inline bool m_loaded{ false };
};
} // namespace kogayonon_utilities
