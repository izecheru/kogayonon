#include "core/scene/scene_manager.h"
#include "core/scene/scene.h"

namespace kogayonon_core
{
SceneManager& SceneManager::getInstance()
{
    static SceneManager instance{};
    return instance;
}

void kogayonon_core::SceneManager::addScene(std::shared_ptr<Scene> scene, const std::string& name)
{
    m_scenes.emplace(name, std::move(scene));
}

void SceneManager::removeScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it != m_scenes.end())
    {
        m_scenes.erase(it);
    }
}

std::weak_ptr<Scene> SceneManager::getCurrentScene()
{
    return std::weak_ptr<Scene>(m_scenes.at(m_currentScene));
}

void SceneManager::setCurrentScene(const std::string& sceneName)
{
    m_currentScene = sceneName;
}
} // namespace kogayonon_core