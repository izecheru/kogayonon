#pragma once
#include <memory>

#include "asset_manager/asset_manager.h"
#include "event/event_manager.h"
#include "registry.h"
#include "renderer/renderer.h"
#include "task/task_manager.h"
#include "window/window.h"

namespace kogayonon
{
// i used entt instead of implementing my own stuff just for the sake of speed and all the other features
#define REGISTRY() RegistryManager::getInstance()
#define EVENT_MANAGER() RegistryManager::getInstance().getContext<std::shared_ptr<EventManager>>()
#define ASSET_MANAGER() RegistryManager::getInstance().getContext<std::shared_ptr<AssetManager>>()
#define TASK_MANAGER() RegistryManager::getInstance().getContext<std::shared_ptr<TaskManager>>()
#define RENDERER() RegistryManager::getInstance().getContext<std::shared_ptr<Renderer>>()

/**
 * @brief RegistryManager holds the main parts of the application
 */
class RegistryManager
{
public:
  inline static RegistryManager& getInstance()
  {
    static RegistryManager instance{};
    return instance;
  }

  template <typename TContext>
  TContext& addToContext(TContext context)
  {
    return m_registry->addToContext<TContext>(context);
  }

  template <typename TContext>
  TContext& getContext()
  {
    return m_registry->getContext<TContext>();
  }

  void initialise(std::shared_ptr<Window> window);
  Renderer& getRenderer();
  TaskManager& getTaskManager();
  EventManager& getEventManager();
  AssetManager& getAssetManager();

private:
  RegistryManager() = default;
  ~RegistryManager() = default;
  RegistryManager(const RegistryManager&) = delete;
  RegistryManager& operator=(const RegistryManager&) = delete;

  inline static std::shared_ptr<Registry> m_registry;
  inline static bool m_init = false;
};
} // namespace kogayonon
