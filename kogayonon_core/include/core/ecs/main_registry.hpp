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
// i used entt instead of implementing my own stuff just for the sake of speed and all the other features
#define REGISTRY() kogayonon_core::MainRegistry::getInstance()
#define EVENT_DISPATCHER() REGISTRY().getContext<std::shared_ptr<kogayonon_core::EventDispatcher>>()
#define IMGUI_MANAGER() REGISTRY().getContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>()
#define TASK_MANAGER() REGISTRY().getContext<std::shared_ptr<kogayonon_utilities::TaskManager>>()
#define ASSET_MANAGER() REGISTRY().getContext<std::shared_ptr<kogayonon_utilities::AssetManager>>()
#define TIME_TRACKER() REGISTRY().getContext<std::shared_ptr<kogayonon_utilities::TimeTracker>>()
#define RENDERER() REGISTRY().getContext<std::shared_ptr<kogayonon_rendering::Renderer>>()

// this should be in the asset manager
#define SHADER_MANAGER() REGISTRY().getContext<std::shared_ptr<kogayonon_utilities::ShaderManager>>()

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

private:
  MainRegistry() = default;
  ~MainRegistry() = default;
  MainRegistry( const MainRegistry& ) = delete;
  MainRegistry& operator=( const MainRegistry& ) = delete;

  inline static std::shared_ptr<Registry> m_pRegistry;
  inline static bool m_init = false;
};
} // namespace kogayonon_core