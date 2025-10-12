#pragma once
#include <string>

namespace kogayonon_core
{
struct IdentifierComponent
{
  std::string name{};
  // TODO those should be some kind of enum, will do for now
  std::string type{};
  std::string group{};
};
} // namespace kogayonon_core