#pragma once
#include <glm/glm.hpp>
#include "resources/model.hpp"

namespace kogayonon_core
{
struct ModelComponent
{
  explicit ModelComponent( std::weak_ptr<kogayonon_resources::Model> model )
      : pModel{ model }
      , loaded{ true }
  {
  }

  std::weak_ptr<kogayonon_resources::Model> pModel;
  bool loaded;
};
} // namespace kogayonon_core
