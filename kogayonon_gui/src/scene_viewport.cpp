#include "gui/scene_viewport.h"
#include <glad/glad.h>
#include "rendering/framebuffer.h"

namespace kogayonon_gui
{
SceneViewportWindow::SceneViewportWindow(std::string name) : ImGuiWindow(std ::move(name))
{
    m_pFrameBuffer = std::make_unique<kogayonon_rendering::FrameBuffer>(m_props->width, m_props->height);
}

kogayonon_rendering::FrameBuffer* SceneViewportWindow::getFrameBuffer()
{
    return m_pFrameBuffer.get();
}

void SceneViewportWindow::draw()
{
    if (!ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->flags))
    {
        ImGui::End();
        return;
    }

    m_props->width = ImGui::GetContentRegionAvail().x;
    m_props->height = ImGui::GetContentRegionAvail().y;

    // draw to framebuffer
    int fbWidth = (int)m_props->width;
    int fbHeight = (int)m_props->height;
    m_pFrameBuffer->bind();
    m_pFrameBuffer->rescale(fbWidth, fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(0.0, 1.0, 0.0, 0.2);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_renderCallback)
    {
        m_renderCallback();
    }
    m_pFrameBuffer->unbind();

    ImVec2 win_pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddImage((void*)m_pFrameBuffer->getTexture(), ImVec2(win_pos.x, win_pos.y),
                                         ImVec2(win_pos.x + m_props->width, win_pos.y + m_props->height), ImVec2(0, 1),
                                         ImVec2(1, 0));

    ImGui::End();
}
} // namespace kogayonon_gui