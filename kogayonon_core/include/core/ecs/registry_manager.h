#pragma once
#include <memory>
#include "registry.h"

namespace kogayonon_utilities {
class TaskManager;
}

namespace kogayonon_gui {
class ImGuiManager;
}

namespace kogayonon_core {

// forward declaration
class EventManager;

// i used entt instead of implementing my own stuff just for the sake of speed and all the other features
#define REGISTRY() kogayonon_core::RegistryManager::getInstance()
#define EVENT_MANAGER() kogayonon_core::RegistryManager::getInstance().getContext<std::shared_ptr<kogayonon_core::EventManager>>()
#define TASK_MANAGER() kogayonon_core::RegistryManager::getInstance().getContext<std::shared_ptr<kogayonon_utilities::TaskManager>>()
#define IMGUI_MANAGER() kogayonon_core::RegistryManager::getInstance().getContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>()

/**
 * @brief RegistryManager holds the main parts of the application
 */
class RegistryManager
{
  public:
    inline static RegistryManager& getInstance()
    {
        static RegistryManager instance{};
        if (!m_init)
        {
            m_registry = std::make_shared<Registry>();
            m_init = true;
        }
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

  private:
    RegistryManager() = default;
    ~RegistryManager() = default;
    RegistryManager(const RegistryManager&) = delete;
    RegistryManager& operator=(const RegistryManager&) = delete;

    inline static std::shared_ptr<Registry> m_registry;
    inline static bool m_init = false;
};
} // namespace kogayonon_core
