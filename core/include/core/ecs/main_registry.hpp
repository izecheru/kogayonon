#pragma once
#include <memory>
#include "registry.hpp"

namespace utilities
{
class TaskManager;
class ShaderManager;
class TimeTracker;
} // namespace utilities

namespace kogayonon_gui
{
class ImGuiManager;
} // namespace kogayonon_gui

namespace core
{
// this struct holds the main lua functions
struct MainScriptFuncs;

class EventEmitter;
class EventDispatcher;
class ScriptingSystem;
} // namespace core

namespace kogayonon_rendering
{
class Renderer;
}

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
  auto addToContext( TContext context ) -> TContext&
  {
    return m_pRegistry->addToContext<TContext>( context );
  }

  template <typename TContext>
  auto getContext() -> TContext&
  {
    return m_pRegistry->getContext<TContext>();
  }

  auto getMainScriptFuncs() -> std::shared_ptr<core::MainScriptFuncs>&
  {
    return getContext<std::shared_ptr<core::MainScriptFuncs>>();
  }

  auto getEventEmitter() -> std::shared_ptr<core::EventEmitter>&
  {
    return getContext<std::shared_ptr<core::EventEmitter>>();
  }

  auto getEventDispatcher() -> std::shared_ptr<core::EventDispatcher>&
  {
    return getContext<std::shared_ptr<core::EventDispatcher>>();
  }

  auto getImGuiManager() -> std::shared_ptr<kogayonon_gui::ImGuiManager>&
  {
    return getContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>();
  }

  auto getTaskManager() -> std::shared_ptr<utilities::TaskManager>&
  {
    return getContext<std::shared_ptr<utilities::TaskManager>>();
  }

  auto getTimeTracker() -> std::shared_ptr<utilities::TimeTracker>&
  {
    return getContext<std::shared_ptr<utilities::TimeTracker>>();
  }

  auto getShaderManager() -> std::shared_ptr<utilities::ShaderManager>&
  {
    return getContext<std::shared_ptr<utilities::ShaderManager>>();
  }

  auto getScriptingSystem() -> std::shared_ptr<core::ScriptingSystem>&
  {
    return getContext<std::shared_ptr<core::ScriptingSystem>>();
  }

private:
  MainRegistry() = default;
  ~MainRegistry() = default;
  MainRegistry( const MainRegistry& ) = delete;
  MainRegistry& operator=( const MainRegistry& ) = delete;

  inline static std::shared_ptr<Registry> m_pRegistry;
  inline static bool m_init = false;
};
} // namespace core