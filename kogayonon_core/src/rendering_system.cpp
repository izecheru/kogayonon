#include "core/systems/rendering_system.h"
#include <assert.h>
#include <entt/entt.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/scene/scene.hpp"
#include "utilities/math/math.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
using namespace kogayonon_utilities;

namespace kogayonon_core
{
void RenderingSystem::render( std::shared_ptr<Scene> scene, glm::mat4& viewMatrix, glm::mat4& projection,
                              kogayonon_utilities::Shader& shader )
{
  begin( shader );

  auto view = scene->getEnttRegistry().view<TransformComponent, ModelComponent>();

  for ( const auto& [entity, transformComp, modelComp] : view.each() )
  {
  }

  for ( const auto& [entity, transformComp, modelComp] : view.each() )
  {
    if ( !modelComp.loaded )
      continue;

    auto model = modelComp.pModel.lock();

    if ( !model )
      continue;

    // then draw
    shader.setMat4( "projection", projection );
    shader.setMat4( "view", viewMatrix );

    for ( auto& mesh : modelComp.pModel.lock()->getMeshes() )
    {
      glBindVertexArray( mesh.getVao() );

      const auto& textures = mesh.getTextures();
      for ( const auto& texture : textures )
      {
        // bind the texture we need (this is bad)
        glBindTextureUnit( 1, texture );
      }

      // this will help us determine what type of rendering we have so the vertex shader knows which matrices to
      // multiply , shader.setBool( "instanced", true );
      if ( auto instanceData = scene->getData( model.get() ); instanceData != nullptr && instanceData->count > 1 )
      {
        shader.setBool( "instanced", true );

        // sort instance data based on Z position

        // draw the instances
        glDrawElementsInstanced( GL_TRIANGLES, (GLsizei)mesh.getIndices().size(), GL_UNSIGNED_INT, nullptr,
                                 instanceData->count );
      }
      else
      {
        shader.setBool( "instanced", false );
        transformComp.updateMatrix();
        shader.setMat4( "model", transformComp.modelMatrix );

        // draw the indices
        glDrawElements( GL_TRIANGLES, (GLsizei)mesh.getIndices().size(), GL_UNSIGNED_INT, nullptr );
      }

      // unbind everything
      glBindVertexArray( 0 );
      glBindTextureUnit( 1, 0 );
    }
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