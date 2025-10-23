#include "core/systems/rendering_system.h"
#include <assert.h>
#include <entt/entt.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "resources/model.hpp"
#include "utilities/math/math.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
using namespace kogayonon_utilities;

namespace kogayonon_core
{
void RenderingSystem::render( std::shared_ptr<Scene> scene, glm::mat4& viewMatrix, glm::mat4& projection,
                              kogayonon_utilities::Shader& shader ) const
{
  begin( shader );

  auto view = scene->getEnttRegistry().view<TransformComponent, ModelComponent, IndexComponent>();

  std::unordered_map<kogayonon_resources::Model*, int> orderedModels;
  std::unordered_set<kogayonon_resources::Model*> uniqueModels;

  for ( const auto& [entity, transformComp, modelComp, indexComp] : view.each() )
  {
    if ( !modelComp.loaded )
      continue;

    auto model = modelComp.pModel;
    if ( !model )
      continue;

    // only true if first seen
    if ( uniqueModels.insert( model ).second )
      orderedModels.emplace( model, indexComp.index );
  }

  shader.setMat4( "projection", projection );
  shader.setMat4( "view", viewMatrix );
  for ( auto& model : orderedModels )
  {
    if ( !model.first )
      continue;

    for ( auto& mesh : model.first->getMeshes() )
    {
      glBindVertexArray( mesh.getVao() );

      const auto& textures = mesh.getTextures();
      for ( const auto& texture : textures )
      {
        // bind the texture we need (this is bad)
        glBindTextureUnit( 1, texture->getTextureId() );
      }

      auto instanceData = scene->getData( model.first );
      if ( instanceData != nullptr )
      {
        // draw the instances
        glDrawElementsInstanced( GL_TRIANGLES, (GLsizei)mesh.getIndices().size(), GL_UNSIGNED_INT, nullptr,
                                 instanceData->count );
      }
      // unbind everything
      glBindVertexArray( 0 );
      glBindTextureUnit( 1, 0 );
    }
  }
  end( shader );
}

void RenderingSystem::render( kogayonon_resources::Model* model, glm::mat4& viewMatrix, glm::mat4& projection,
                              kogayonon_utilities::Shader& shader ) const
{
  begin( shader );

  auto& meshes = model->getMeshes();

  for ( auto& mesh : meshes )
  {
    glBindVertexArray( mesh.getVao() );

    const auto& textures = mesh.getTextures();
    for ( const auto& texture : textures )
    {
      // bind the texture we need (this is bad)
      glBindTextureUnit( 1, texture->getTextureId() );
    }

    auto instanceData = SceneManager::getCurrentScene().lock()->getData( model );
    if ( instanceData != nullptr )
    {
      // draw the instances
      glDrawElementsInstanced( GL_TRIANGLES, (GLsizei)mesh.getIndices().size(), GL_UNSIGNED_INT, nullptr,
                               instanceData->count );
    }
    // unbind everything
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