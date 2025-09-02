#include "registry_manager.h"

namespace kogayonon
{
void RegistryManager::initialise(std::shared_ptr<Window> window)
{
  m_registry = std::make_unique<Registry>();
  assert(m_registry && "Failed to initialize the registry!");

  auto renderer = std::make_shared<Renderer>(window);
  assert(renderer && "could not initialize renderer");
  addToContext<std::shared_ptr<Renderer>>(std::move(renderer));

  auto event_manager = std::make_shared<EventManager>();
  assert(event_manager && "could not initialize event manager");
  addToContext<std::shared_ptr<EventManager>>(std::move(event_manager));

  auto task_manager = std::make_shared<TaskManager>(10);
  assert(task_manager && "could not initialize task manager");
  addToContext<std::shared_ptr<TaskManager>>(std::move(task_manager));

  auto asset_manager = std::make_shared<AssetManager>();
  assert(asset_manager && "could not initialize asset manager");
  addToContext<std::shared_ptr<AssetManager>>(std::move(asset_manager));

  m_init = true;
  KLogger::critical("Registries initialised");
}

Renderer& RegistryManager::getRenderer()
{
  assert(m_init && "Registries must be initialised before use");
  return *m_registry->getContext<std::shared_ptr<Renderer>>();
}

TaskManager& RegistryManager::getTaskManager()
{
  assert(m_init && "Registries must be initialised before use");
  return *m_registry->getContext<std::shared_ptr<TaskManager>>();
}

EventManager& RegistryManager::getEventManager()
{
  assert(m_init && "Registries must be initialised before use");
  return *m_registry->getContext<std::shared_ptr<EventManager>>();
}

AssetManager& RegistryManager::getAssetManager()
{
  assert(m_init && "Registries must be initialised before use");
  return *m_registry->getContext<std::shared_ptr<AssetManager>>();
}
} // namespace kogayonon