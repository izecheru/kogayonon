#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

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

// sort renderable objects based on camera pos,
// if let's say we have a house of a said z and then another smaller house with a z that's
// behind the big house, we don't render the object, probably will have to look into drawing just
// the right meshes but will get to that at some point in time
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