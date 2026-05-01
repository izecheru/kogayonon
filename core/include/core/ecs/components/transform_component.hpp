#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sol/sol.hpp>
#include <yaml-cpp/yaml.h>
#include "utilities/json_serializer/json_serializer.hpp"
#include "utilities/utils/yaml_utils.hpp"

namespace core
{

struct TransformComponent
{
  glm::vec3 translation{ 0.0f };
  glm::vec3 rotation{ 0.0f };
  glm::vec3 scale{ 1.0f };
  glm::mat4 modelMatrix{ 1.0f };
  bool update{ false };

  /**
   * @brief Calculate the model matrix based on transform data and mark as updated
   */
  void computeMatrix()
  {
    modelMatrix = glm::translate( { 1.0f }, translation );

    modelMatrix = glm::rotate( modelMatrix, rotation.x, glm::vec3{ 1.0f, 0.0f, 0.0f } );
    modelMatrix = glm::rotate( modelMatrix, rotation.y, glm::vec3{ 0.0f, 1.0f, 0.0f } );
    modelMatrix = glm::rotate( modelMatrix, rotation.z, glm::vec3{ 0.0f, 0.0f, 1.0f } );

    modelMatrix = glm::scale( modelMatrix, scale );

    update = false;
  }

  auto getMatrix() -> glm::mat4&
  {
    return modelMatrix;
  }

  static void createLuaBindings( sol::state& lua )
  {
    lua.new_usertype<TransformComponent>( "TransformComponent",
                                          "typeId",
                                          entt::type_hash<TransformComponent>::value,
                                          sol::call_constructor,
                                          sol::factories(
                                            []( glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale ) {
                                              return TransformComponent{
                                                .translation = translation, .rotation = rotation, .scale = scale };
                                            },
                                            []() { return TransformComponent{}; } ),

                                          "translation",
                                          &TransformComponent::translation,
                                          "rotation",
                                          &TransformComponent::rotation,
                                          "scale",
                                          &TransformComponent::scale );
  }
};

} // namespace core
