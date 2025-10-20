#pragma once
#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

namespace kogayonon_utilities
{
/**
 * @brief Could not come up with a better name -_-
 */
class Jsoner
{
public:
  static void parseJsonFile( rapidjson::Document& docRef, const std::filesystem::path& path );
  static rapidjson::Document parseJsonFile( const std::filesystem::path& path );
  static void writeJsonFile( rapidjson::Document& docRef, const std::filesystem::path& path );

  static bool checkObject( const rapidjson::Value& toCheck, const std::string& member );
  static bool checkArray( const rapidjson::Value& toCheck, const std::string& member );

private:
  Jsoner() = delete;
  Jsoner( const Jsoner& ) = delete;
  Jsoner& operator=( const Jsoner& ) = delete;
};
} // namespace kogayonon_utilities