#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <yaml-cpp/yaml.h>
#include "resources/pointlight.hpp"
#include "utilities/utils/yaml_utils.hpp"

namespace kogayonon_core
{
struct PointLightComponent
{
  uint32_t pointLightIndex{ 0u };

  static void createLuaBindings( sol::state& lua )
  {
    lua.new_usertype<PointLightComponent>(
      "PointLightComponent",
      "typeId",
      entt::type_hash<PointLightComponent>::value,
      sol::call_constructor,
      sol::factories( []( uint32_t index ) { return PointLightComponent{ .pointLightIndex = index }; } ),
      "pointLightIndex",
      &PointLightComponent::pointLightIndex );
  }
};
} // namespace kogayonon_core

namespace YAML
{

inline Emitter& operator<<( Emitter& out, const kogayonon_resources::PointLight& point )
{
  out << YAML::BeginMap;
  KEY_VALUE( "color", point.color );
  KEY_VALUE( "ambient", point.ambient );
  KEY_VALUE( "params", point.params );
  KEY_VALUE( "specular", point.specular );
  KEY_VALUE( "diffuse", point.diffuse );
  KEY_VALUE( "translation", point.translation );
  out << YAML::EndMap;
  return out;
}

template <>
struct convert<kogayonon_resources::PointLight>
{
  static Node encode( const kogayonon_resources::PointLight& rhs )
  {
    Node node;
    node["ambient"] = rhs.ambient;
    node["color"] = rhs.color;
    node["diffuse"] = rhs.diffuse;
    node["params"] = rhs.params;
    node["specular"] = rhs.specular;
    node["translation"] = rhs.translation;
    return node;
  }

  static bool decode( const Node& node, kogayonon_resources::PointLight& rhs )
  {
    rhs.ambient = node["ambient"].as<glm::vec4>();
    rhs.color = node["color"].as<glm::vec4>();
    rhs.diffuse = node["diffuse"].as<glm::vec4>();
    rhs.params = node["params"].as<glm::vec4>();
    rhs.specular = node["specular"].as<glm::vec4>();
    rhs.translation = node["translation"].as<glm::vec4>();
    return true;
  }
};

} // namespace YAML
