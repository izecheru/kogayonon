#include "utilities/configurator/configurator.hpp"
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

namespace kogayonon_utilities
{
void Configurator::parseConfigFile()
{
  if ( !std::filesystem::exists( m_configPath.string() ) )
  {
    spdlog::info( "Config file does not exist, writing a default one" );
    initialiseDefaultConfig();
  }

  std::ifstream ifs{ m_configPath.string() };

  rapidjson::IStreamWrapper isw{ ifs };

  m_jsonDocument.ParseStream( isw );

  // now build the config struct
  buildConfig();
}

void Configurator::writeConfig()
{
  std::ofstream ofs{ m_configPath.string() };
  if ( !ofs.is_open() )
  {
    spdlog::error( "could not open config for writing" );
    return;
  }

  rapidjson::OStreamWrapper osw{ ofs };
  rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer{ osw };
  m_jsonDocument.Accept( writer );
  ofs.close();
  spdlog::info( "Wrote config to {}", m_configPath.string() );
}

rapidjson::Document& Configurator::getDocument()
{
  return m_jsonDocument;
}

Config& Configurator::getConfig()
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
  if ( !checkObject( m_jsonDocument, "config" ) )
    return;

  const auto& config = m_jsonDocument["config"];

  if ( !checkObject( config, "filters" ) )
    return;

  const auto& filters = config["filters"];

  if ( !checkArray( filters, "files" ) )
    return;

  // push back all the file filters entries
  const auto& fileFilters = filters["files"];
  for ( rapidjson::SizeType i = 0; i < fileFilters.Size(); i++ )
  {
    m_config.fileFilters.push_back( fileFilters[i].GetString() );
  }

  if ( !checkArray( filters, "folders" ) )
    return;

  // push back all the file filters entries
  const auto& folderFilters = filters["folders"];
  for ( rapidjson::SizeType i = 0; i < folderFilters.Size(); i++ )
  {
    m_config.folderFilters.push_back( folderFilters[i].GetString() );
  }

  if ( !checkObject( config, "window" ) )
    return;

  const auto& window = config["window"];
  m_config.height = window["height"].GetInt();
  m_config.width = window["width"].GetInt();
  m_config.maximized = window["maximized"].GetBool();

  m_loaded = true;
}

bool Configurator::checkObject( const rapidjson::Value& toCheck, const std::string& member )
{
  if ( toCheck.HasMember( member.c_str() ) && toCheck[member.c_str()].IsObject() )
    return true;

  return false;
}

bool Configurator::checkArray( const rapidjson::Value& toCheck, const std::string& member )
{
  if ( toCheck.HasMember( member.c_str() ) && toCheck[member.c_str()].IsArray() )
    return true;

  return false;
}

} // namespace kogayonon_utilities