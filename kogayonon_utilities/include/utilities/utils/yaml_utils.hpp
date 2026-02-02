#pragma once
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>
#include "resources/pointlight.hpp"

namespace YAML
{
#define KEY_VALUE( key, value ) out << YAML::Key << key << YAML::Value << value

template <>
struct convert<glm::vec3>
{
  static Node encode( const glm::vec3& rhs )
  {
    Node node;
    node.push_back( rhs.x );
    node.push_back( rhs.y );
    node.push_back( rhs.z );
    node.SetStyle( EmitterStyle::Flow );
    return node;
  }

  static bool decode( const Node& node, glm::vec3& rhs )
  {
    if ( !node.IsSequence() || node.size() != 3 )
      return false;

    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    rhs.z = node[2].as<float>();
    return true;
  }
};

template <>
struct convert<glm::vec4>
{
  static Node encode( const glm::vec4& rhs )
  {
    Node node;
    node.push_back( rhs.x );
    node.push_back( rhs.y );
    node.push_back( rhs.z );
    node.push_back( rhs.w );
    node.SetStyle( EmitterStyle::Flow );
    return node;
  }

  static bool decode( const Node& node, glm::vec4& rhs )
  {
    if ( !node.IsSequence() || node.size() != 4 )
      return false;

    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    rhs.z = node[2].as<float>();
    rhs.w = node[3].as<float>();
    return true;
  }
};

inline Emitter& operator<<( Emitter& out, const glm::vec3& vec )
{
  Node node;
  node["vec3"] = convert<glm::vec3>::encode( vec );
  return out;
}

inline Emitter& operator<<( Emitter& out, const glm::vec4& vec )
{
  Node node;
  node["vec4"] = convert<glm::vec4>::encode( vec );
  return out;
}

} // namespace YAML