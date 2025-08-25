#include "renderer/renderer.h"

namespace kogayonon
{
void Renderer::draw()
{
  setRenderCallback([this]() { callback_test(); });
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_imgui_manager->draw();
  m_window->update();
}

bool Renderer::getPolyMode()
{
  return is_poly;
}

void Renderer::togglePolyMode()
{
  is_poly = !is_poly;

  switch (is_poly)
  {
  case true:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;

  case false:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  }
}
} // namespace kogayonon