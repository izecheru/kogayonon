#pragma once
#include "physics/rigidbody_type.hpp"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/EActivation.h>
#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace core
{

struct RigidbodyData
{
  physics::RigidbodyType type;
  physics::RigidbodyShape shape;
  uint32_t layer{};
  JPH::EMotionType motionType{};
  JPH::EActivation activation{};
};

struct RigidbodyComponent
{
  RigidbodyData data;
  JPH::BodyID body;
};
} // namespace core