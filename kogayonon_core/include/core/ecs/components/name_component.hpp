#pragma once
#include <string>

namespace kogayonon_core
{
struct NameComponent
{
  explicit NameComponent( const std::string& n )
      : name{ n }
  {
  }

  ~NameComponent() = default;

  std::string name;
};
} // namespace kogayonon_core