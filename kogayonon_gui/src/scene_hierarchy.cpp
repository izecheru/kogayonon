#include "gui/scene_hierarchy.h"
#include "core/ecs/components/texture_component.h"
#include "core/ecs/entity.h"
#include "core/ecs/registry.h"
#include "core/scene/scene.h"
#include "core/scene/scene_manager.h"
#include "resources/texture.h"

namespace kogayonon_gui
{
SceneHierarchyWindow::SceneHierarchyWindow(std::string name) : ImGuiWindow(name)
{
}

void SceneHierarchyWindow::draw()
{
    if (!ImGui::Begin(m_props->name.c_str(), nullptr, m_props->flags))
    {
        ImGui::End();
        return;
    }

    auto& sceneManager = kogayonon_core::SceneManager::getInstance();
    auto& currentScene = sceneManager.getCurrentScene();
    auto& scene = currentScene.lock();
    if (scene)
    {
        auto& registry = scene->getRegistry();
        auto& enttRegistry = registry.getRegistry();
        for (auto& [entity, textureComponent] : enttRegistry.view<kogayonon_core::TextureComponent>().each())
        {
            ImGui::Text("Texture path: %s ", textureComponent.m_texture.lock()->getPath().c_str());
        }
    }

    ImGui::End();
}
} // namespace kogayonon_gui