#pragma once

namespace physics
{
enum RigidbodyType
{
  Static,
  Dynamic
};

enum RigidbodyShape
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