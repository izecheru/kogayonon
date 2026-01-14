#include "utilities/configurator/configurator.hpp"
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>
#include "utilities/jsoner/jsoner.hpp"

namespace kogayonon_utilities
{
void Configurator::parseConfigFile()
{
  if ( !std::filesystem::exists( m_configPath ) )
  {
    initialiseDefaultConfig();
  }
  Jsoner::parseJsonFile( m_jsonDocument, m_configPath );
  buildConfig();
}

void Configurator::writeConfig()
{
  Jsoner::writeJsonFile( m_jsonDocument, m_configPath );
}

rapidjson::Document& Configurator::getDocument()
{
  return m_jsonDocument;
}

auto Configurator::getConfig() -> Config&
{
  assert( m_loaded && "document was not loaded correctly" );
  return m_config;
}

void Configurator::initialiseDefaultConfig()
{
  m_jsonDocument.SetObject();
  auto& allocator = m_jsonDocument.GetAllocator();

  rapidjson::Value configObj{ rapidjson::kObjectType };
  rapidjson::Value filtersObj{ rapidjson::kObjectType };
  rapidjson::Value windowObj{ rapidjson::kObjectType };

  rapidjson::Value filesArray{ rapidjson::kArrayType };
  filesArray.PushBack( rapidjson::Value( ".bin", allocator ), allocator );

  rapidjson::Value foldersArray{ rapidjson::kArrayType };
  foldersArray.PushBack( rapidjson::Value( "fonts", allocator ), allocator );

  filtersObj.AddMember( "files", filesArray, allocator );
  filtersObj.AddMember( "folders", foldersArray, allocator );

  windowObj.AddMember( "width", 1800, allocator );
  windowObj.AddMember( "height", 900, allocator );
  windowObj.AddMember( "maximized", true, allocator );

  configObj.AddMember( "filters", filtersObj, allocator );
  configObj.AddMember( "window", windowObj, allocator );

  m_jsonDocument.AddMember( "config", configObj, allocator );

  writeConfig();
}

void Configurator::buildConfig()
{
  if ( !Jsoner::checkObject( m_jsonDocument, "config" ) )
    return;

  const auto& config = m_jsonDocument["config"];

  if ( !Jsoner::checkObject( config, "filters" ) )
    return;

  const auto& filters = config["filters"];

  if ( !Jsoner::checkArray( filters, "files" ) )
    return;

  // push back all the file filters entries
  const auto& fileFilters = filters["files"];
  for ( rapidjson::SizeType i = 0; i < fileFilters.Size(); i++ )
  {
    m_config.fileFilters.push_back( fileFilters[i].GetString() );
  }

  if ( !Jsoner::checkArray( filters, "folders" ) )
    return;

  // push back all the file filters entries
  const auto& folderFilters = filters["folders"];
  for ( rapidjson::SizeType i = 0; i < folderFilters.Size(); i++ )
  {
    m_config.folderFilters.push_back( folderFilters[i].GetString() );
  }

  if ( !Jsoner::checkObject( config, "window" ) )
    return;

  const auto& window = config["window"];
  m_config.height = window["height"].GetInt();
  m_config.width = window["width"].GetInt();
  m_config.maximized = window["maximized"].GetBool();

  spdlog::info( "Config file read, w {} h {} max {}", m_config.width, m_config.height, m_config.maximized );

  m_loaded = true;
}
} // namespace kogayonon_utilities