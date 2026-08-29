#pragma once
#include "physics/rigidbody_type.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <queue>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>

#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyID.h>

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <Jolt/RegisterTypes.h>

namespace Layers
{
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::uint NUM_LAYERS = 2;
}; // namespace Layers

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
static constexpr JPH::BroadPhaseLayer NON_MOVING( 0 );
static constexpr JPH::BroadPhaseLayer MOVING( 1 );
static constexpr JPH::uint NUM_LAYERS( 2 );
}; // namespace BroadPhaseLayers

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
  virtual bool ShouldCollide( JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2 ) const override
  {
    switch ( inObject1 )
    {
    case Layers::NON_MOVING:
      return inObject2 == Layers::MOVING; // Non moving only collides with moving
    case Layers::MOVING:
      return true; // Moving collides with everything
    default:
      JPH_ASSERT( false );
      return false;
    }
  }
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
  BPLayerInterfaceImpl()
  {
    // Create a mapping table from object to broad phase layer
    mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
  }

  virtual JPH::uint GetNumBroadPhaseLayers() const override
  {
    return BroadPhaseLayers::NUM_LAYERS;
  }

  virtual JPH::BroadPhaseLayer GetBroadPhaseLayer( JPH::ObjectLayer inLayer ) const override
  {
    JPH_ASSERT( inLayer < Layers::NUM_LAYERS );
    return mObjectToBroadPhase[inLayer];
  }

private:
  JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
  virtual bool ShouldCollide( JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2 ) const override
  {
    switch ( inLayer1 )
    {
    case Layers::NON_MOVING:
      return inLayer2 == BroadPhaseLayers::MOVING;
    case Layers::MOVING:
      return true;
    default:
      JPH_ASSERT( false );
      return false;
    }
  }
};

// An example contact listener
class MyContactListener : public JPH::ContactListener
{
public:
  // See: ContactListener
  virtual JPH::ValidateResult OnContactValidate( const JPH::Body& inBody1,
                                                 const JPH::Body& inBody2,
                                                 JPH::RVec3Arg inBaseOffset,
                                                 const JPH::CollideShapeResult& inCollisionResult ) override
  {

    // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
  }

  virtual void OnContactAdded( const JPH::Body& inBody1,
                               const JPH::Body& inBody2,
                               const JPH::ContactManifold& inManifold,
                               JPH::ContactSettings& ioSettings ) override
  {
  }

  virtual void OnContactPersisted( const JPH::Body& inBody1,
                                   const JPH::Body& inBody2,
                                   const JPH::ContactManifold& inManifold,
                                   JPH::ContactSettings& ioSettings ) override
  {
  }

  virtual void OnContactRemoved( const JPH::SubShapeIDPair& inSubShapePair ) override
  {
  }
};

// An example activation listener
class MyBodyActivationListener : public JPH::BodyActivationListener
{
public:
  virtual void OnBodyActivated( const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData ) override
  {
  }

  virtual void OnBodyDeactivated( const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData ) override
  {
  }
};

namespace physics
{

class JoltPhysics
{
public:
  JoltPhysics();
  ~JoltPhysics();

  void onUpdate( float delta );

  bool isRunning() const;
  auto getPhysicsSystem() -> JPH::PhysicsSystem&;

  void start();
  void stop();

  auto createRigidBody( const RigidbodyType& type,
                        const RigidbodyShape& shape,
                        const glm::vec3& pos,
                        const glm::vec3& size,
                        const glm::quat& rotation ) -> JPH::BodyID;

  void deactivateBody( JPH::BodyID& body );
  void activateBody( JPH::BodyID& body );
  bool isBodyActive( JPH::BodyID& body );

private:
  float m_isRunning;
  float m_deltaUpdate;
  JPH::PhysicsSystem m_physicsSystem;
  std::unique_ptr<JPH::TempAllocatorImpl> m_tempAlloc;
  std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;

  BPLayerInterfaceImpl m_broadPhaseLayerInterface;
  ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadphaseLayerFilter;
  MyBodyActivationListener m_bodyActivationListener;
  MyContactListener m_contactListener;
  ObjectLayerPairFilterImpl m_objectVsLayerFilter;

  std::queue<std::function<void()>> m_deletionQueue;

  float m_timeAccumulator;
};
} // namespace physics