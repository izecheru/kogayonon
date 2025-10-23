#pragma once
#include "resources/model.hpp"

namespace kogayonon_core
{
struct ModelComponent
{
  kogayonon_resources::Model* pModel{ nullptr };
  bool loaded{ false };
};
} // namespace kogayonon_core