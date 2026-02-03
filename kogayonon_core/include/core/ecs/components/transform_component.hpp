#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <yaml-cpp/yaml.h>
#include "utilities/json_serializer/json_serializer.hpp"
#include "utilities/utils/yaml_utils.hpp"

namespace kogayonon_core
{
struct TransformComponent
{
  glm::vec3 translation{ 0.0f };
  glm::vec3 rotation{ 0.0f };
  glm::vec3 scale{ 1.0f };

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

} // namespace kogayonon_core

namespace YAML
{

template <>
struct convert<kogayonon_core::TransformComponent>
{
  static Node encode( const kogayonon_core::TransformComponent& rhs )
  {
    Node node;
    node["translation"] = rhs.translation;
    node["rotation"] = rhs.rotation;
    node["scale"] = rhs.scale;
    return node;
  }

  static bool decode( const Node& node, kogayonon_core::TransformComponent& transform )
  {
    if ( !node["translation"] || !node["rotation"] || !node["scale"] )
      return false;

    transform.translation = node["translation"].as<glm::vec3>();
    transform.rotation = node["rotation"].as<glm::vec3>();
    transform.scale = node["scale"].as<glm::vec3>();
    return true;
  }
};

inline Emitter& operator<<( YAML::Emitter& out, const kogayonon_core::TransformComponent& transform )
{
  out << convert<kogayonon_core::TransformComponent>::encode( transform );
  return out;
}

} // namespace YAML