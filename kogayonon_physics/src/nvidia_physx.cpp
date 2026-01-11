#include "physics/nvidia_physx.hpp"
#include <assert.h>
#include <spdlog/spdlog.h>

namespace kogayonon_physics
{

NvidiaPhysx::NvidiaPhysx()
    : m_foundation{ nullptr }
    , m_physics{ nullptr }
    , m_pvd{ nullptr }
    , m_scene{ nullptr }
    , m_dispatcher{ nullptr }
    , m_material{ nullptr }
    , m_isRunning{ false }
{
  initPhysx();
}

bool NvidiaPhysx::isRunning() const
{
  return m_isRunning;
}

void NvidiaPhysx::simulate( float delta )
{
  if ( !m_isRunning )
    return;

  m_scene->simulate( delta );
  fetchResults( true );
}

void NvidiaPhysx::switchState( bool state )
{
  m_isRunning = state;
}

void NvidiaPhysx::fetchResults( bool block )
{
  m_scene->fetchResults( true );
}

void NvidiaPhysx::initPhysx()
{
  m_foundation = PxCreateFoundation( PX_PHYSICS_VERSION, g_defaultAllocatorCallback, g_defaultErrorCallback );
  assert( m_foundation && "Could not initialise NvidiaPhysx Foundation" );
  spdlog::debug( "NvidiaPhysx foundation was initialised!" );

  m_pvd = PxCreatePvd( *m_foundation );
  physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate( PVD_HOST, 5425, 10 );
  m_pvd->connect( *transport, physx::PxPvdInstrumentationFlag::eALL );
  m_scale.length = 100;
  m_scale.speed = 90;
  m_physics = PxCreatePhysics( PX_PHYSICS_VERSION, *m_foundation, m_scale, true, m_pvd );
  physx::PxSceneDesc sceneDesc{ m_physics->getTolerancesScale() };
  sceneDesc.gravity = physx::PxVec3{ 0.0f, -9.81f, 0.0f };
  m_dispatcher = physx::PxDefaultCpuDispatcherCreate( 2 );
  sceneDesc.cpuDispatcher = m_dispatcher;
  sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
  m_scene = m_physics->createScene( sceneDesc );

  physx::PxPvdSceneClient* pvdClient = m_scene->getScenePvdClient();
  if ( pvdClient )
  {
    pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true );
    pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true );
    pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true );
  }
  m_material = m_physics->createMaterial( 0.5f, 0.5f, 0.6f );
}

void NvidiaPhysx::releasePhysx()
{
  PX_RELEASE( m_scene );
  PX_RELEASE( m_dispatcher );
  PX_RELEASE( m_physics );
  PX_RELEASE( m_pvd );
  PX_RELEASE( m_foundation );
}

auto NvidiaPhysx::getPhysics() -> physx::PxPhysics*
{
  return m_physics;
}

auto NvidiaPhysx::getMaterial() -> physx::PxMaterial*
{
  return m_material;
}

auto NvidiaPhysx::getScene() -> physx::PxScene*
{
  return m_scene;
}
} // namespace kogayonon_physics
