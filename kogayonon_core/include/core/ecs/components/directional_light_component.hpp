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
template <>
struct convert<kogayonon_core::DirectionalLightComponent>
{
  static Node encode( const kogayonon_core::DirectionalLightComponent& directionalLightComp )
  {
    Node node;
    node["nearPlane"] = directionalLightComp.nearPlane;
    node["farPlane"] = directionalLightComp.farPlane;
    node["orthoSize"] = directionalLightComp.orthoSize;
    node["positionFactor"] = directionalLightComp.positionFactor;
    return node;
  }

  static bool decode( const Node& node, kogayonon_core::DirectionalLightComponent& directionalLightComp )
  {
    directionalLightComp.nearPlane = node["nearPlane"].as<float>();
    directionalLightComp.farPlane = node["farPlane"].as<float>();
    directionalLightComp.positionFactor = node["positionFactor"].as<float>();
    directionalLightComp.orthoSize = node["orthoSize"].as<float>();
    return true;
  }
};

template <>
struct convert<kogayonon_resources::DirectionalLight>
{
  static Node encode( const kogayonon_resources::DirectionalLight& directionalLight )
  {
    Node node;
    node["diffuse"] = directionalLight.diffuse;
    node["specular"] = directionalLight.specular;
    node["direction"] = directionalLight.direction;
    return node;
  }

  static bool decode( const Node& node, kogayonon_resources::DirectionalLight& direcitonalLight )
  {
    direcitonalLight.diffuse = node["diffuse"].as<glm::vec4>();
    direcitonalLight.specular = node["specular"].as<glm::vec4>();
    direcitonalLight.direction = node["direction"].as<glm::vec4>();
    return true;
  }
};

inline Emitter& operator<<( YAML::Emitter& out, const kogayonon_resources::DirectionalLight& directionalLight )
{
  out << convert<kogayonon_resources::DirectionalLight>::encode( directionalLight );
  return out;
}

inline Emitter& operator<<( YAML::Emitter& out,
                            const kogayonon_core::DirectionalLightComponent& directionalLightComponent )
{
  out << convert<kogayonon_core::DirectionalLightComponent>::encode( directionalLightComponent );
  return out;
}

} // namespace YAML
