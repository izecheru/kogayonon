#include "core/systems/rendering_system.h"
#include <entt/entt.hpp>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "rendering/camera/camera.hpp"
#include "resources/vertex.hpp"
#include "utilities/shader_manager/shader_manager.hpp"

namespace kogayonon_core
{

void RenderingSystem::render( int w, int h, kogayonon_rendering::Camera* camera, kogayonon_utilities::Shader& shader )
{
  begin( shader );

  auto scene = SceneManager::getCurrentScene();
  auto pScene = scene.lock();

  // if scene does not exist return
  if ( !pScene )
    return;

  auto view = pScene->getEnttRegistry().view<TransformComponent, ModelComponent>();
  for ( auto [entity, transformComp, modelComp] : view.each() )
  {
    Entity ent{ pScene->getRegistry(), entity };
    auto& model = ent.getComponent<ModelComponent>();

    if ( !model.loaded )
      continue;

    // then draw
    glm::mat4 viewMat = camera->getViewMatrix();
    glm::mat4 modelMat = computeModelMatrix( transformComp );
    glm::mat4 proj = glm::perspective( glm::radians( 45.0f ), (float)w / (float)h, 0.1f, 4000.0f );
    shader.setMat4( "projection", proj );
    shader.setMat4( "view", viewMat );
    shader.setMat4( "model", modelMat );
    for ( auto& mesh : model.pModel.lock()->getMeshes() )
    {
      auto& textures = mesh.getTextures();
      for ( int i = 0; i < textures.size(); i++ )
      {
        glBindTextureUnit( 1, textures.at( i ) );
      }
      glBindVertexArray( mesh.getVao() );
      glDrawElements( GL_TRIANGLES, (GLsizei)mesh.getIndices().size(), GL_UNSIGNED_INT, nullptr );
      glBindVertexArray( 0 );
      glBindTextureUnit( 1, 0 );
    }
  }
  end( shader );
}

void RenderingSystem::begin( kogayonon_utilities::Shader& shader ) const
{
  shader.bind();
}

void RenderingSystem::end( kogayonon_utilities::Shader& shader ) const
{
  shader.unbind();
}

glm::mat4 RenderingSystem::computeModelMatrix( TransformComponent& transform ) const
{
  glm::mat4 model( 1.0f );

  // translate
  model = glm::translate( model, transform.pos );

  // rotate (Z * Y * X)
  model = glm::rotate( model, transform.rotation.z, glm::vec3( 0, 0, 1 ) );
  model = glm::rotate( model, transform.rotation.y, glm::vec3( 0, 1, 0 ) );
  model = glm::rotate( model, transform.rotation.x, glm::vec3( 1, 0, 0 ) );

  // scale
  model = glm::scale( model, transform.scale );

  return model;
}

} // namespace kogayonon_core