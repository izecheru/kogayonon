#pragma once
#include <memory>
#include "registry.hpp"

namespace kogayonon_utilities
{
class TaskManager;
class ShaderManager;
class AssetManager;
class TimeTracker;
} // namespace kogayonon_utilities

namespace kogayonon_gui
{
class ImGuiManager;
} // namespace kogayonon_gui

namespace kogayonon_core
{
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
  TContext& addToContext( TContext context )
  {
    return m_pRegistry->addToContext<TContext>( context );
  }

  template <typename TContext>
  TContext& getContext()
  {
    return m_pRegistry->getContext<TContext>();
  }

  std::shared_ptr<kogayonon_core::EventDispatcher>& getEventDispatcher()
  {
    return getContext<std::shared_ptr<kogayonon_core::EventDispatcher>>();
  }

  std::shared_ptr<kogayonon_gui::ImGuiManager>& getImGuiManager()
  {
    return getContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>();
  }

  std::shared_ptr<kogayonon_utilities::TaskManager>& getTaskManager()
  {
    return getContext<std::shared_ptr<kogayonon_utilities::TaskManager>>();
  }

  std::shared_ptr<kogayonon_utilities::AssetManager>& getAssetManager()
  {
    return getContext<std::shared_ptr<kogayonon_utilities::AssetManager>>();
  }

  std::shared_ptr<kogayonon_utilities::TimeTracker>& getTimeTracker()
  {
    return getContext<std::shared_ptr<kogayonon_utilities::TimeTracker>>();
  }

  std::shared_ptr<kogayonon_rendering::Renderer>& getRenderer()
  {
    return getContext<std::shared_ptr<kogayonon_rendering::Renderer>>();
  }

  std::shared_ptr<kogayonon_utilities::ShaderManager>& getShaderManager()
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