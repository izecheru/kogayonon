#pragma once
#include "resources/model.hpp"

namespace kogayonon_core
{
struct ModelComponent
{
  std::weak_ptr<kogayonon_resources::Model> pModel;
  bool loaded;
};
} // namespace kogayonon_core