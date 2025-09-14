#pragma once

namespace kogayonon_rendering
{
class Renderer
{
  public:
    Renderer() = default;
    ~Renderer() = default;

    void draw();

    bool getPoly();
    void togglePoly();

  private:
    bool m_bPolyMode = false;
};
} // namespace kogayonon_rendering
