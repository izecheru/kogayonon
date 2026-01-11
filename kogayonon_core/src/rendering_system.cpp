
#include "core/systems/rendering_system.h"
#include <assert.h>
#include <entt/entt.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "rendering/opengl_framebuffer.hpp"
#include "utilities/math/math.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
using namespace kogayonon_utilities;

namespace kogayonon_core
{
void RenderingSystem::renderDepthPass( FrameContext& frame, DepthPassContext& pass )
{
  beginDepthPass( frame.canvas );
  render( frame.scene, frame.view, frame.projection, pass.shader );
  endDepthPass( frame.canvas );
}

void RenderingSystem::renderGeometryPass( FrameContext& frame, GeometryPassContext& pass )
{
  beginGeometryPass( frame.canvas );

  frame.scene->bindLightBuffers();

  renderWithDepth( frame.scene, frame.view, frame.projection, pass.lightVP, pass.shader, pass.depthMap );

  frame.scene->unbindLightBuffers();

  endGeometryPass( frame.canvas );
}

auto RenderingSystem::renderPickingPass( FrameContext& frame, PickingPassContext& pass ) -> int
{
  auto& framebuffer = frame.canvas.framebuffer;
  beginPickingPass( frame.canvas );

  render( frame.scene, frame.view, frame.projection, pass.shader );

  auto result = framebuffer.readPixel( 0, pass.x, pass.y );

  endPickingPass( frame.canvas );

  return result;
}

void RenderingSystem::render( Scene* scene, glm::mat4& viewMatrix, glm::mat4& projection,
                              kogayonon_utilities::Shader& shader )
{
  begin( shader );

  scene->bindLightBuffers();

  std::unordered_map<kogayonon_resources::Mesh*, int> orderedMeshes;
  makeMeshesUnique( scene, orderedMeshes );

  shader.setMat4( "projection", projection );
  shader.setMat4( "view", viewMatrix );
  drawMeshes( scene, orderedMeshes );
  scene->unbindLightBuffers();
  end( shader );
}

void RenderingSystem::drawMeshesWithDepth( Scene* scene,
                                           const std::unordered_map<kogayonon_resources::Mesh*, int>& orderedMeshes,
                                           const uint32_t& depthMap )
{
  for ( auto& mesh : orderedMeshes )
  {
    if ( !mesh.first )
      continue;

    glBindVertexArray( mesh.first->getVao() );
    const auto& textures = mesh.first->getTextures();
    for ( int i = 0; i < textures.size(); i++ )
    {
      // bind the texture we need (this is bad)
      glBindTextureUnit( 3, textures.at( i )->getTextureId() );
    }
    glBindTextureUnit( 4, depthMap );

    auto instanceData = scene->getData( mesh.first );
    auto& submeshes = mesh.first->getSubmeshes();
    for ( int i = 0; i < submeshes.size() && instanceData != nullptr; i++ )
    {

      glDrawElementsInstancedBaseVertex( GL_TRIANGLES, submeshes.at( i ).indexCount, GL_UNSIGNED_INT,
                                         (void*)( submeshes.at( i ).indexOffset * sizeof( uint32_t ) ),
                                         instanceData->count, submeshes.at( i ).vertexOffest );
    }
    glBindVertexArray( 0 );
    glBindTextureUnit( 1, 0 );
  }
}

void RenderingSystem::drawMeshes( Scene* scene,
                                  const std::unordered_map<kogayonon_resources::Mesh*, int>& orderedMeshes )
{
  for ( auto& mesh : orderedMeshes )
  {
    if ( !mesh.first )
      continue;

    glBindVertexArray( mesh.first->getVao() );
    const auto& textures = mesh.first->getTextures();
    for ( int i = 0; i < textures.size(); i++ )
    {
      // bind the texture we need (this is bad)
      glBindTextureUnit( 3, textures.at( i )->getTextureId() );
    }

    auto instanceData = scene->getData( mesh.first );
    auto& submeshes = mesh.first->getSubmeshes();
    for ( int i = 0; i < submeshes.size() && instanceData != nullptr; i++ )
    {

      glDrawElementsInstancedBaseVertex( GL_TRIANGLES, submeshes.at( i ).indexCount, GL_UNSIGNED_INT,
                                         (void*)( submeshes.at( i ).indexOffset * sizeof( uint32_t ) ),
                                         instanceData->count, submeshes.at( i ).vertexOffest );
    }
    glBindVertexArray( 0 );
    glBindTextureUnit( 1, 0 );
  }
}

void RenderingSystem::renderWithDepth( Scene* scene, glm::mat4& viewMatrix, glm::mat4& projection,
                                       glm::mat4& lightSpaceMatrix, kogayonon_utilities::Shader& shader,
                                       uint32_t& depthMap )
{
  begin( shader );

  scene->bindLightBuffers();

  std::unordered_map<kogayonon_resources::Mesh*, int> orderedMeshes;
  makeMeshesUnique( scene, orderedMeshes );

  shader.setMat4( "projection", projection );
  shader.setMat4( "view", viewMatrix );
  shader.setMat4( "lightVP", lightSpaceMatrix );

  drawMeshesWithDepth( scene, orderedMeshes, depthMap );

  scene->unbindLightBuffers();
  end( shader );
}

void RenderingSystem::renderGeometryWithShadows( std::shared_ptr<Scene> scene, const glm::mat4& viewMatrix,
                                                 const glm::mat4& projection, kogayonon_utilities::Shader& shader,
                                                 const glm::mat4& lightProjectionView, const uint32_t depthMap )
{
  begin( shader );
  scene->bindLightBuffers();

  std::unordered_map<kogayonon_resources::Mesh*, int> orderedMeshes;
  makeMeshesUnique( scene.get(), orderedMeshes );

  shader.setMat4( "projection", projection );
  shader.setMat4( "view", viewMatrix );
  shader.setMat4( "lightVP", lightProjectionView );
  for ( auto& mesh : orderedMeshes )
  {
    if ( !mesh.first )
      continue;

    glBindVertexArray( mesh.first->getVao() );
    const auto& textures = mesh.first->getTextures();
    for ( int i = 0; i < textures.size(); i++ )
    {
      // bind the texture we need (this is bad)
      glBindTextureUnit( 3, textures.at( i )->getTextureId() );
    }
    glBindTextureUnit( 4, depthMap );

    auto instanceData = scene->getData( mesh.first );
    auto& submeshes = mesh.first->getSubmeshes();
    for ( int i = 0; i < submeshes.size() && instanceData != nullptr; i++ )
    {

      glDrawElementsInstancedBaseVertex( GL_TRIANGLES, submeshes.at( i ).indexCount, GL_UNSIGNED_INT,
                                         (void*)( submeshes.at( i ).indexOffset * sizeof( uint32_t ) ),
                                         instanceData->count, submeshes.at( i ).vertexOffest );
    }
    glBindVertexArray( 0 );
    glBindTextureUnit( 1, 0 );
  }
  scene->unbindLightBuffers();
  end( shader );
}

void RenderingSystem::begin( const kogayonon_utilities::Shader& shader ) const
{
  glUseProgram( 0 );
  shader.bind();
}

void RenderingSystem::end( const kogayonon_utilities::Shader& shader ) const
{
  shader.unbind();
}

void RenderingSystem::beginGeometryPass( Canvas& canvas ) const
{
  auto& framebuffer = canvas.framebuffer;
  framebuffer.resize( canvas.w, canvas.h );
  framebuffer.bind();

  glClearColor( 0.3f, 0.3f, 0.3f, 0.5f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void RenderingSystem::endGeometryPass( Canvas& canvas ) const
{
  auto& framebuffer = canvas.framebuffer;
  framebuffer.unbind();
}

void RenderingSystem::beginDepthPass( Canvas& canvas ) const
{
  auto& framebuffer = canvas.framebuffer;

  glCullFace( GL_FRONT );
  framebuffer.resize( canvas.w, canvas.h );
  framebuffer.bind();

  glClear( GL_DEPTH_BUFFER_BIT );
}

void RenderingSystem::endDepthPass( Canvas& canvas ) const
{
  auto& framebuffer = canvas.framebuffer;
  framebuffer.unbind();
  glCullFace( GL_BACK );
}

void RenderingSystem::beginPickingPass( Canvas& canvas ) const
{
  auto& framebuffer = canvas.framebuffer;

  framebuffer.resize( canvas.w, canvas.h );
  framebuffer.bind();
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  framebuffer.clearColorAttachment( 0, -1 );
}

void RenderingSystem::endPickingPass( Canvas& canvas ) const
{
  auto& framebuffer = canvas.framebuffer;
  framebuffer.unbind();
}

void RenderingSystem::makeMeshesUnique( Scene* scene,
                                        std::unordered_map<kogayonon_resources::Mesh*, int>& orderedMeshes )
{
  auto view = scene->getEnttRegistry().view<TransformComponent, MeshComponent, IndexComponent>();
  std::unordered_set<kogayonon_resources::Mesh*> uniqueMeshes;

  for ( const auto& [entity, transformComp, meshComponent, indexComp] : view.each() )
  {
    if ( !meshComponent.loaded )
      continue;

    auto& mesh = meshComponent.pMesh;

    if ( !mesh )
      continue;

    // only true if first seen
    if ( uniqueMeshes.insert( mesh ).second )
      orderedMeshes.emplace( mesh, indexComp.index );
  }
}
} // namespace kogayonon_core
