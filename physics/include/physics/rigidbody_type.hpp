#pragma once

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
} // namespace physics