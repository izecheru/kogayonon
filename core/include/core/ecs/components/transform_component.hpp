#pragma once
#define GLM_FORCE_RADIANS
#include "utilities/json_serializer/json_serializer.hpp"
#include "utilities/utils/yaml_utils.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sol/sol.hpp>
#include <yaml-cpp/yaml.h>

namespace core
{

struct TransformComponent
{
  glm::vec3 translation{ 0.0f };
  glm::vec3 rotation{ 0.0f };
  glm::vec3 scale{ 1.0f };
  glm::mat4 modelMatrix{ 1.0f };

  /**
   * @brief Get a quaternion from the vec3 rotation
   * @return
   */
  [[nodiscard]] inline auto getOrientation() const -> glm::quat
  {
    return glm::quat{ glm::radians( rotation ) };
  }

  /**
   * @brief Calculate the model matrix based on transform data and mark as updated
   */
  void computeMatrix()
  {
    modelMatrix = glm::translate( { 1.0f }, translation );

    modelMatrix *= glm::mat4_cast( getOrientation() );

    modelMatrix = glm::scale( modelMatrix, scale );
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
