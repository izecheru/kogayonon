#include "physics/jolt_physics.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>
using namespace JPH;

static void TraceImpl( const char* inFMT, ... )
{
  va_list list;
  va_start( list, inFMT );
  char buffer[1024];
  vsnprintf( buffer, sizeof( buffer ), inFMT, list );
  va_end( list );

  spdlog::error( buffer );
}

physics::JoltPhysics::JoltPhysics()
    : m_deltaUpdate( 1.0f / 60.0f )
    , m_isRunning{ true }
    , m_timeAccumulator{ 0.0f }
{
  RegisterDefaultAllocator();

  Trace = TraceImpl;
  Factory::sInstance = new Factory();
  RegisterTypes();

  m_tempAlloc = std::make_unique<JPH::TempAllocatorImpl>( 50 * 1024 * 1024 );

  m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
    cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1 );

  const uint cMaxBodies = 1024;
  const uint cNumBodyMutexes = 0;
  const uint cMaxBodyPairs = 1024;
  const uint cMaxContactConstraints = 1024;

  m_physicsSystem.Init( cMaxBodies,
                        cNumBodyMutexes,
                        cMaxBodyPairs,
                        cMaxContactConstraints,
                        m_broadPhaseLayerInterface,
                        m_objectVsBroadphaseLayerFilter,
                        m_objectVsLayerFilter );

  m_physicsSystem.SetBodyActivationListener( &m_bodyActivationListener );
  m_physicsSystem.SetContactListener( &m_contactListener );
  m_physicsSystem.SetGravity( { 0.0f, -3.5f, 0.0f } );
}

physics::JoltPhysics::~JoltPhysics()
{
  while ( !m_deletionQueue.empty() )
  {
    auto& func = m_deletionQueue.front();
    func();

    m_deletionQueue.pop();
  }
  UnregisterTypes();
  delete Factory::sInstance;
  Factory::sInstance = nullptr;
}

void physics::JoltPhysics::update( float delta )
{
  if ( !m_isRunning )
    return;

  m_timeAccumulator += delta;

  while ( m_timeAccumulator >= m_deltaUpdate )
  {
    m_physicsSystem.Update( m_deltaUpdate, 1, m_tempAlloc.get(), m_jobSystem.get() );
    m_timeAccumulator -= m_deltaUpdate;
  }
}

bool physics::JoltPhysics::isRunning() const
{
  return m_isRunning;
}

auto physics::JoltPhysics::getPhysicsSystem() -> JPH::PhysicsSystem&
{
  return m_physicsSystem;
}

void physics::JoltPhysics::start()
{
  m_isRunning = true;
}

void physics::JoltPhysics::stop()
{
  m_isRunning = false;
}

auto physics::JoltPhysics::createRigidBody( const RigidbodyType& type,
                                            const RigidbodyShape& shape,
                                            const glm::vec3& pos,
                                            const glm::vec3& size,
                                            const glm::quat& rotation ) -> BodyID
{
  auto& interface = m_physicsSystem.GetBodyInterface();
  JPH::BodyID r{};

  switch ( shape )
  {
    // Box shape, both static and dynamic
  case RigidbodyShape::Box: {
    JPH::Ref<JPH::BoxShapeSettings> boxSettings = new JPH::BoxShapeSettings( Vec3{ size.x, size.y, size.z } );

    JPH::ShapeSettings::ShapeResult shapeResult = boxSettings->Create();
    JPH::Ref<JPH::Shape> s = shapeResult.Get();

    uint32_t layer{};
    EMotionType motionType{};

    if ( type == RigidbodyType::Dynamic )
    {
      layer = Layers::MOVING;
      motionType = EMotionType::Dynamic;
    }
    else
    {
      layer = Layers::NON_MOVING;
      motionType = EMotionType::Static;
    }

    BodyCreationSettings settings(
      s, RVec3{ pos.x, pos.y, pos.z }, Quat{ rotation.x, rotation.y, rotation.z, rotation.w }, motionType, layer );

    r = interface.CreateAndAddBody( settings, EActivation::Activate );

    m_deletionQueue.push( [this, r]() {
      auto& itf = m_physicsSystem.GetBodyInterface();
      itf.RemoveBody( r );
      itf.DestroyBody( r );
    } );

    break;
  }
  }

  return r;
}

void physics::JoltPhysics::deactivateBody( JPH::BodyID& body )
{
  auto& bodyInterface = m_physicsSystem.GetBodyInterface();
  bodyInterface.DeactivateBody( body );
}

void physics::JoltPhysics::activateBody( JPH::BodyID& body )
{
  auto& bodyInterface = m_physicsSystem.GetBodyInterface();
  bodyInterface.ActivateBody( body );
}

bool physics::JoltPhysics::isBodyActive( JPH::BodyID& body )
{
  auto& bodyInterface = m_physicsSystem.GetBodyInterface();
  return bodyInterface.IsActive( body );
}
