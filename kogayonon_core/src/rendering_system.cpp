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
#include "utilities/math/math.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
using namespace kogayonon_utilities;

namespace kogayonon_core
{
void RenderingSystem::render( std::shared_ptr<Scene> scene, glm::mat4& viewMatrix, glm::mat4& projection,
                              kogayonon_utilities::Shader& shader ) const
{
  begin( shader );

  static glm::mat4 biasMatrix{ 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0 };
  auto view = scene->getEnttRegistry().view<TransformComponent, MeshComponent, IndexComponent>();

  std::unordered_map<kogayonon_resources::Mesh*, int> orderedMeshes;
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

  shader.setMat4( "projection", projection );
  shader.setMat4( "view", viewMatrix );
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
  end( shader );
}

void RenderingSystem::renderGeometryWithShadows( std::shared_ptr<Scene> scene, const glm::mat4& viewMatrix,
                                                 const glm::mat4& projection, kogayonon_utilities::Shader& shader,
                                                 const glm::mat4& lightProjectionView, const uint32_t depthMap )
{
  begin( shader );

  auto view = scene->getEnttRegistry().view<TransformComponent, MeshComponent, IndexComponent>();

  std::unordered_map<kogayonon_resources::Mesh*, int> orderedMeshes;
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
} // namespace kogayonon_core