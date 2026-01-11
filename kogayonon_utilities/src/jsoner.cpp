#include "utilities/jsoner/jsoner.hpp"
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include <rapidjson/prettywriter.h>

using namespace rapidjson;

namespace kogayonon_utilities
{
auto Jsoner::parseJsonFile( const std::filesystem::path& path ) -> Document
{
  assert( std::filesystem::exists( path ) && "Json file does not exist!!!" );

  std::fstream ifs{ path.string(), std::ios::in };

  IStreamWrapper isw{ ifs };

  Document doc{};
  doc.ParseStream( isw );

  if ( ifs )
    ifs.close();

  return doc;
}

void Jsoner::parseJsonFile( Document& docRef, const std::filesystem::path& path )
{
  assert( std::filesystem::exists( path ) && "Json file does not exist!!!" );

  std::fstream ifs{ path.string(), std::ios::in };

  IStreamWrapper isw{ ifs };

  docRef.ParseStream( isw );

  if ( ifs )
    ifs.close();
}

void Jsoner::writeJsonFile( Document& docRef, const std::filesystem::path& path )
{
  std::fstream ofs{ path.string(), std::ios::out };

  OStreamWrapper osw{ ofs };
  PrettyWriter<OStreamWrapper> writer{ osw };

  docRef.Accept( writer );

  if ( ofs )
    ofs.close();
}

bool Jsoner::checkObject( const Value& toCheck, const std::string& member )
{
  if ( toCheck.HasMember( member.c_str() ) && toCheck[member.c_str()].IsObject() )
    return true;

  return false;
}

bool Jsoner::checkArray( const Value& toCheck, const std::string& member )
{
  if ( toCheck.HasMember( member.c_str() ) && toCheck[member.c_str()].IsArray() )
    return true;

  return false;
}
} // namespace kogayonon_utilities