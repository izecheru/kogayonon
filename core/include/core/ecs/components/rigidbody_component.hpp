#pragma once
#include "physics/rigidbody_type.hpp"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace core
{

struct RigidbodyComponent
{
  physics::RigidbodyType type;
  JPH::BodyID body;
};
} // namespace core