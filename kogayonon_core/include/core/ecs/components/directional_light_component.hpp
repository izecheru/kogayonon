#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <yaml-cpp/yaml.h>
#include "resources/directional_light.hpp"
#include "utilities/utils/yaml_utils.hpp"

namespace kogayonon_core
{
struct DirectionalLightComponent
{
  int directionalLightIndex{ 0 };
  float nearPlane{ 0.1f };
  float farPlane{ 300.0f };
  float orthoSize{ 70.0f };
  float positionFactor{ 20.0f };

  static void createLuaBindings( sol::state& lua )
  {
    lua.new_usertype<DirectionalLightComponent>(
      "DirectionalLightComponent",
      "typeId",
      entt::type_hash<DirectionalLightComponent>::value,
      sol::call_constructor,
      sol::factories( []() { return DirectionalLightComponent{}; },
                      []( float near, float far, float orthoSize, float posFactor ) {
                        return DirectionalLightComponent{
                          .nearPlane = near, .farPlane = far, .orthoSize = orthoSize, .positionFactor = posFactor };
                      } ),
      "directionalLightIndex",
      &DirectionalLightComponent::directionalLightIndex,
      "nearPlane",
      &DirectionalLightComponent::nearPlane,
      "farPlane",
      &DirectionalLightComponent::farPlane,
      "orthoSize",
      &DirectionalLightComponent::orthoSize,
      "positionFactor",
      &DirectionalLightComponent::positionFactor );
  }
};
} // namespace kogayonon_core

namespace YAML
{

inline Emitter& operator<<( YAML::Emitter& out, const kogayonon_resources::DirectionalLight& t )
{
  out << YAML::BeginMap;
  KEY_VALUE( "diffuse", t.diffuse );
  KEY_VALUE( "specular", t.specular );
  KEY_VALUE( "direction", t.direction );
  out << YAML::EndMap;

  return out;
}

inline Emitter& operator<<( YAML::Emitter& out, const kogayonon_core::DirectionalLightComponent& t )
{
  out << YAML::BeginMap;
  KEY_VALUE( "orthoSize", t.orthoSize );
  KEY_VALUE( "nearPlane", t.nearPlane );
  KEY_VALUE( "farPlane", t.farPlane );
  KEY_VALUE( "positionFactor", t.positionFactor );
  out << YAML::EndMap;

  return out;
}

template <>
struct convert<kogayonon_core::DirectionalLightComponent>
{
  static Node encode( const kogayonon_core::DirectionalLightComponent& rhs )
  {
    Node node;
    node["nearPlane"] = rhs.nearPlane;
    node["farPlane"] = rhs.farPlane;
    node["orthoSize"] = rhs.orthoSize;
    node["positionFactor"] = rhs.positionFactor;
    return node;
  }

  static bool decode( const Node& node, kogayonon_core::DirectionalLightComponent& rhs )
  {
    rhs.nearPlane = node["nearPlane"].as<float>();
    rhs.farPlane = node["farPlane"].as<float>();
    rhs.positionFactor = node["positionFactor"].as<float>();
    rhs.orthoSize = node["orthoSize"].as<float>();
    return true;
  }
};

template <>
struct convert<kogayonon_resources::DirectionalLight>
{
  static Node encode( const kogayonon_resources::DirectionalLight& rhs )
  {
    Node node;
    node["diffuse"] = rhs.diffuse;
    node["specular"] = rhs.specular;
    node["direction"] = rhs.direction;
    return node;
  }

  static bool decode( const Node& node, kogayonon_resources::DirectionalLight& rhs )
  {
    rhs.diffuse = node["diffuse"].as<glm::vec4>();
    rhs.specular = node["specular"].as<glm::vec4>();
    rhs.direction = node["direction"].as<glm::vec4>();
    return true;
  }
};

} // namespace YAML
