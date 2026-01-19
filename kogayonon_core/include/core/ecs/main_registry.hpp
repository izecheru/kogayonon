#pragma once
#include <memory>
#include "registry.hpp"

namespace kogayonon_utilities
{
class TaskManager;
class ShaderManager;
class TimeTracker;
} // namespace kogayonon_utilities

namespace kogayonon_gui
{
class ImGuiManager;
} // namespace kogayonon_gui

namespace kogayonon_core
{
class EventEmitter;
class EventDispatcher;
} // namespace kogayonon_core

namespace kogayonon_rendering
{
class Renderer;
}

namespace kogayonon_core
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

  auto getEventEmitter() -> std::shared_ptr<kogayonon_core::EventEmitter>&
  {
    return getContext<std::shared_ptr<kogayonon_core::EventEmitter>>();
  }

  auto getEventDispatcher() -> std::shared_ptr<kogayonon_core::EventDispatcher>&
  {
    return getContext<std::shared_ptr<kogayonon_core::EventDispatcher>>();
  }

  auto getImGuiManager() -> std::shared_ptr<kogayonon_gui::ImGuiManager>&
  {
    return getContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>();
  }

  auto getTaskManager() -> std::shared_ptr<kogayonon_utilities::TaskManager>&
  {
    return getContext<std::shared_ptr<kogayonon_utilities::TaskManager>>();
  }

  auto getTimeTracker() -> std::shared_ptr<kogayonon_utilities::TimeTracker>&
  {
    return getContext<std::shared_ptr<kogayonon_utilities::TimeTracker>>();
  }

  auto getRenderer() -> std::shared_ptr<kogayonon_rendering::Renderer>&
  {
    return getContext<std::shared_ptr<kogayonon_rendering::Renderer>>();
  }

  auto getShaderManager() -> std::shared_ptr<kogayonon_utilities::ShaderManager>&
  {
    return getContext<std::shared_ptr<kogayonon_utilities::ShaderManager>>();
  }

private:
  MainRegistry() = default;
  ~MainRegistry() = default;
  MainRegistry( const MainRegistry& ) = delete;
  MainRegistry& operator=( const MainRegistry& ) = delete;

  inline static std::shared_ptr<Registry> m_pRegistry;
  inline static bool m_init = false;
};
} // namespace kogayonon_core