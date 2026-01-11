#pragma once
#include <physx/PxPhysics.h>
#include <physx/PxScene.h>
#include <physx/common/PxTolerancesScale.h>
#include <physx/extensions/PxDefaultAllocator.h>
#include <physx/extensions/PxDefaultCpuDispatcher.h>
#include <physx/extensions/PxDefaultErrorCallback.h>
#include <physx/extensions/PxDefaultSimulationFilterShader.h>
#include <physx/foundation/PxFoundation.h>
#include <physx/foundation/PxPhysicsVersion.h>
#include <physx/pvd/PxPvd.h>
#include <physx/pvd/PxPvdTransport.h>

namespace kogayonon_physics
{
constexpr const char* PVD_HOST = "127.0.0.1";
static physx::PxDefaultErrorCallback g_defaultErrorCallback;
static physx::PxDefaultAllocator g_defaultAllocatorCallback;

class NvidiaPhysx
{
public:
  inline static NvidiaPhysx& getInstance()
  {
    static NvidiaPhysx instance{};
    return instance;
  }

  void releasePhysx();
  auto isRunning() const -> bool;
  void simulate( float delta );
  void fetchResults( bool block );

  /**
   * @brief Flips m_isSimulating flag to mark the start of physics simulation and to also stop it
   */
  void switchState( bool state );

  auto getPhysics() -> physx::PxPhysics*;
  auto getMaterial() -> physx::PxMaterial*;
  auto getScene() -> physx::PxScene*;

private:
  void initPhysx();

  // copy is not allowed
  NvidiaPhysx( const NvidiaPhysx& ) = delete;
  NvidiaPhysx& operator=( const NvidiaPhysx& ) = delete;
  NvidiaPhysx( NvidiaPhysx&& ) = delete;
  NvidiaPhysx& operator=( NvidiaPhysx&& ) = delete;

  NvidiaPhysx();
  ~NvidiaPhysx() = default;

  physx::PxFoundation* m_foundation;
  physx::PxPhysics* m_physics;
  physx::PxPvd* m_pvd;
  physx::PxTolerancesScale m_scale;
  physx::PxScene* m_scene;
  physx::PxDefaultCpuDispatcher* m_dispatcher;
  physx::PxMaterial* m_material;

  bool m_isRunning;
};

} // namespace kogayonon_physics
