#pragma once

namespace kogayonon_rendering {
class FrameBuffer;

class Renderer
{
  public:
    Renderer() = default;
    ~Renderer() = default;

    void draw();

    bool getPoly();
    void togglePoly();

  private:
    FrameBuffer* m_pSceneFrameBuffer;
    bool m_bPolyMode = false;
};
} // namespace kogayonon_rendering
