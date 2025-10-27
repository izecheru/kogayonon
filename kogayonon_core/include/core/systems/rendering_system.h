#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace kogayonon_resources
{
class Model;
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
class Scene;
}

namespace kogayonon_core
{
class RenderingSystem
{
public:
  RenderingSystem() = default;
  ~RenderingSystem() = default;

  void render( std::shared_ptr<Scene> scene, glm::mat4& viewMatrix, glm::mat4& projection,
               kogayonon_utilities::Shader& shader ) const;

  void begin( const kogayonon_utilities::Shader& shader ) const;
  void end( const kogayonon_utilities::Shader& shader ) const;
};
} // namespace kogayonon_core