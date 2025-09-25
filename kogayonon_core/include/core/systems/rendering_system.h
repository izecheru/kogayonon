#pragma once
#include <entt/entt.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <vector>
#include "core/ecs/entity.hpp"
#include "resources/model.hpp"

namespace kogayonon_core
{
class TransformComponent;
}

namespace kogayonon_rendering
{
class Camera;
}

namespace kogayonon_utilities
{
class Shader;
}

namespace kogayonon_core
{
class RenderingSystem
{
public:
  RenderingSystem() = default;
  ~RenderingSystem() = default;

  void render( int w, int h, kogayonon_rendering::Camera* camera, kogayonon_utilities::Shader& shader );
  void begin( kogayonon_utilities::Shader& shader ) const;
  void end( kogayonon_utilities::Shader& shader ) const;

private:
  glm::mat4 computeModelMatrix( TransformComponent& transform ) const;
};
} // namespace kogayonon_core