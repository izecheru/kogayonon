#pragma once

namespace kogayonon_rendering {
class Renderer
{
  public:
    Renderer() = default;
    ~Renderer() = default;

    void draw();

    bool getPolyMode();
    void togglePolyMode();

  private:
    bool is_poly = false;
};
} // namespace kogayonon_rendering
