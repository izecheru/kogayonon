#pragma once
#include <string>

namespace physics
{

enum class RigidbodyType
{
  Static,
  Dynamic
};

enum class RigidbodyShape
{
  Empty,
  Box,
  Sphere,
  Capsule,
  TaperedCapsule,
  Cylinder,
  TaperedCylinder,
  ConvexHull,
  Triangle,
  Plane,
  StaticCompound,
  MutableCompound,
  Mesh
};

inline auto rigidbodyTypeStr( const RigidbodyType& type ) -> std::string
{
  switch ( type )
  {
  case RigidbodyType::Static:
    return "Static";
  case RigidbodyType::Dynamic:
    return "Dynamic";
  default:
    return "Not implemented";
  }
}

inline auto rigidbodyShapeStr( const RigidbodyShape& shape ) -> std::string
{
  switch ( shape )
  {
  case RigidbodyShape::Box:
    return "Box";

  case RigidbodyShape::Sphere:
    return "Sphere";

  default:
    return "Not implemented";
  }
}

} // namespace physics
