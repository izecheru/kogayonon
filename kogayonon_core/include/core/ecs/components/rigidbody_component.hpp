#pragma once
#include <physx/PxRigidDynamic.h>
#include <physx/PxRigidStatic.h>

namespace kogayonon_core
{
struct DynamicRigidbodyComponent
{
  physx::PxRigidDynamic* pBody{ nullptr };
};

struct StaticRigidbodyComponent
{
  physx::PxRigidStatic* pBody{ nullptr };
};
} // namespace kogayonon_core