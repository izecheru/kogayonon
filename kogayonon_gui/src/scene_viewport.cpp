#include "gui/scene_viewport.h"
#include <glad/glad.h>
#include "logger/logger.h"
#include "rendering/framebuffer.h"

namespace kogayonon_gui
{
//      explicit SceneViewportWindow(std::string name, std::shared_ptr<kogayonon_rendering::FrameBuffer> frameBuffer);

SceneViewportWindow::SceneViewportWindow(std::string name,
                                         std::shared_ptr<kogayonon_rendering::FrameBuffer> frameBuffer)
    : ImGuiWindow(std ::move(name)), m_pFrameBuffer(frameBuffer)
{
}

std::weak_ptr<kogayonon_rendering::FrameBuffer> SceneViewportWindow::getFrameBuffer()
{
    return std::weak_ptr<kogayonon_rendering::FrameBuffer>(m_pFrameBuffer);
}

void SceneViewportWindow::draw()
{
    if (!ImGui::Begin(m_props->name.c_str(), nullptr, m_props->flags))
    {
        ImGui::End();
        return;
    }

    auto& pFrameBuffer = m_pFrameBuffer.lock();

    if (pFrameBuffer && m_renderCallback)
    {
        m_props->width = ImGui::GetContentRegionAvail().x;
        m_props->height = ImGui::GetContentRegionAvail().y;

        pFrameBuffer->bind();
        pFrameBuffer->rescale(m_props->width, m_props->height);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw using the callback
        m_renderCallback();

        pFrameBuffer->unbind();

        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddImage((void*)pFrameBuffer->getTexture(), ImVec2(win_pos.x, win_pos.y),
                                             ImVec2(win_pos.x + m_props->width, win_pos.y + m_props->height),
                                             ImVec2(0, 1), ImVec2(1, 0));
    }

    ImGui::End();
}
} // namespace kogayonon_gui