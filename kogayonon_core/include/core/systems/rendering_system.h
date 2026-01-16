#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

namespace kogayonon_rendering
{
class Camera;
class OpenGLFramebuffer;
} // namespace kogayonon_rendering

namespace kogayonon_utilities
{
class Shader;
} // namespace kogayonon_utilities

namespace kogayonon_core
{
class Scene;
} // namespace kogayonon_core

namespace kogayonon_resources
{
class Mesh;
}

namespace kogayonon_core
{
struct Canvas
{
  kogayonon_rendering::OpenGLFramebuffer* framebuffer;
  int w;
  int h;
};

struct FrameContext
{
  Canvas canvas;
  Scene* scene;
  glm::mat4* view;
  glm::mat4* projection;
};

struct OutliningPassContext
{
  kogayonon_utilities::Shader* normalShader;
  kogayonon_utilities::Shader* outlineShader;
  uint32_t& depthMap;
};

struct DepthPassContext
{
  kogayonon_utilities::Shader* shader;
};

struct GeometryPassContext
{
  kogayonon_utilities::Shader* shader;
  uint32_t& depthMap;
  glm::mat4* lightVP;
};

struct PickingPassContext
{
  kogayonon_utilities::Shader* shader;
  int x{ 0 };
  int y{ 0 };
};

class RenderingSystem
{
public:
  RenderingSystem() = default;
  ~RenderingSystem() = default;

  void renderOutliningPass( FrameContext& frame, OutliningPassContext& pass );
  void renderDepthPass( FrameContext& frame, DepthPassContext& pass );
  void renderGeometryPass( FrameContext& frame, GeometryPassContext& pass );
  auto renderPickingPass( FrameContext& frame, PickingPassContext& pass ) -> int;

private:
  void renderOutlinedEntity( Scene* scene, glm::mat4* viewMatrix, glm::mat4* projection,
                             kogayonon_utilities::Shader* shader, uint32_t depthMap );

  void render( Scene* scene, glm::mat4* viewMatrix, glm::mat4* projection, kogayonon_utilities::Shader* shader );

  void renderWithDepth( Scene* scene, glm::mat4* viewMatrix, glm::mat4* projection, glm::mat4* lightSpaceMatrix,
                        kogayonon_utilities::Shader* shader, uint32_t& depthMap );

  void begin( kogayonon_utilities::Shader* shader ) const;
  void end( kogayonon_utilities::Shader* shader ) const;

  void beginOutliningPass( Canvas& canvas ) const;
  void beginGeometryPass( Canvas& canvas ) const;
  void beginPickingPass( Canvas& canvas ) const;
  void beginDepthPass( Canvas& canvas ) const;

  void endOutliningPass( Canvas& canvas ) const;
  void endGeometryPass( Canvas& canvas ) const;
  void endPickingPass( Canvas& canvas ) const;
  void endDepthPass( Canvas& canvas ) const;

  void makeMeshesUnique( Scene* scene, std::unordered_map<kogayonon_resources::Mesh*, int>& orderedMeshes );
  void drawMeshes( Scene* scene, const std::unordered_map<kogayonon_resources::Mesh*, int>& orderedMeshes );

  void drawMeshesWithDepth( Scene* scene, const std::unordered_map<kogayonon_resources::Mesh*, int>& orderedMeshes,
                            const uint32_t& depthMap );
};
} // namespace kogayonon_core
