#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

namespace kogayonon_resources
{
class Model;
} // namespace kogayonon_resources

namespace kogayonon_rendering
{
class Camera;
} // namespace kogayonon_rendering

namespace kogayonon_utilities
{
class Shader;
} // namespace kogayonon_utilities

namespace kogayonon_core
{
class Scene;
} // namespace kogayonon_core

namespace kogayonon_core
{
class RenderingSystem
{
public:
  RenderingSystem() = default;
  ~RenderingSystem() = default;

  void render( std::shared_ptr<Scene> scene, glm::mat4& viewMatrix, glm::mat4& projection,
               kogayonon_utilities::Shader& shader ) const;

  void renderGeometryWithShadows( std::shared_ptr<Scene> scene, const glm::mat4& viewMatrix,
                                  const glm::mat4& projection, kogayonon_utilities::Shader& shader,
                                  const glm::mat4& depthBias, const uint32_t depthMap ) const;

  void begin( const kogayonon_utilities::Shader& shader ) const;
  void end( const kogayonon_utilities::Shader& shader ) const;
};
} // namespace kogayonon_core