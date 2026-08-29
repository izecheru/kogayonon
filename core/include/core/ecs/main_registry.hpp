#pragma once
#include "precompiled/pch.hpp"
#include "registry.hpp"

namespace physics
{
class JoltPhysics;
}

namespace utilities
{
class TaskManager;
class ShaderCompiler;
class TimeTracker;
} // namespace utilities

namespace graphics
{
class VulkanContext;
}

namespace core
{
// this struct holds the main lua functions
struct MainScriptFuncs;
class AssetManager;
class SceneManager;
class EventEmitter;
class EventDispatcher;
class ScriptingSystem;
} // namespace core

namespace core
{
/**
 * @brief MainRegistry holds the main parts of the application
 */
class MainRegistry
{
public:
  inline static MainRegistry& getInstance()
  {
    static MainRegistry instance{};
    if ( !m_init )
    {
      m_pRegistry = std::make_shared<Registry>();
      m_init = true;
    }
    return instance;
  }

  template <typename TContext>
  void addToContext( TContext context )
  {
    m_pRegistry->addToContext<TContext>( context );
  }

  template <typename TContext>
  TContext& getContext()
  {
    return m_pRegistry->getContext<TContext>();
  }

  SceneManager* getSceneManager()
  {
    return getContext<std::shared_ptr<SceneManager>>().get();
  }

  physics::JoltPhysics* getJoltPhysics()
  {
    return getContext<std::shared_ptr<physics::JoltPhysics>>().get();
  }

  graphics::VulkanContext* getVulkanContext()
  {
    return getContext<std::shared_ptr<graphics::VulkanContext>>().get();
  }

  MainScriptFuncs* getMainScriptFuncs()
  {
    return getContext<std::shared_ptr<MainScriptFuncs>>().get();
  }

  EventEmitter* getEventEmitter()
  {
    return getContext<std::shared_ptr<EventEmitter>>().get();
  }

  EventDispatcher* getEventDispatcher()
  {
    return getContext<std::shared_ptr<EventDispatcher>>().get();
  }

  utilities::TimeTracker* getTimeTracker()
  {
    return getContext<std::shared_ptr<utilities::TimeTracker>>().get();
  }

  utilities::TaskManager* getTaskManager()
  {
    return getContext<std::shared_ptr<utilities::TaskManager>>().get();
  }

  utilities::ShaderCompiler* getShaderManager()
  {
    return getContext<std::shared_ptr<utilities::ShaderCompiler>>().get();
  }

  ScriptingSystem* getScriptingSystem()
  {
    return getContext<std::shared_ptr<ScriptingSystem>>().get();
  }

  AssetManager* getAssetManager()
  {
    return getContext<std::shared_ptr<AssetManager>>().get();
  }

private:
  MainRegistry() = default;
  ~MainRegistry();
  MainRegistry( const MainRegistry& ) = delete;
  MainRegistry& operator=( const MainRegistry& ) = delete;

  inline static std::shared_ptr<Registry> m_pRegistry;
  inline static bool m_init = false;
};
} // namespace core