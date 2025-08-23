#include "renderer/renderer.h"

namespace kogayonon
{
  void Renderer::draw() const
  {
    m_imgui_manager->draw();
  }

  bool Renderer::getPolyMode()
  {
    return is_poly;
  }

  void Renderer::togglePolyMode()
  {
    switch (is_poly)
    {
    case true:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;

    case false:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    }

    is_poly = !is_poly;
  }
} // namespace kogayonon