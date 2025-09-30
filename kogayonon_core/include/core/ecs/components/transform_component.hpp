#pragma once
#include "utilities/math/math.hpp"

namespace kogayonon_core
{
struct TransformComponent
{
  glm::vec3 pos{ 0.0f };
  glm::vec3 rotation{ 0.0f };
  glm::vec3 scale{ 1.0f };

  bool dirty{ true };
  glm::mat4 modelMatrix{ 1.0f };

  void setPosition( const glm::vec3& p )
  {
    pos = p;
    dirty = true;
  }

  void setRotation( const glm::vec3& r )
  {
    rotation = r;
    dirty = true;
  }

  void setScale( const glm::vec3& s )
  {
    scale = s;
    dirty = true;
  }

  // recompute only when needed
  void updateMatrix()
  {
    if ( !dirty )
      return;

    modelMatrix = kogayonon_utilities::math::computeModelMatrix( pos, rotation, scale );

    dirty = false;
  }
};
} // namespace kogayonon_core