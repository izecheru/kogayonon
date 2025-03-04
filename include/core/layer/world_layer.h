#pragma once
#include "layer.h"
#include "shader/shader.h"

namespace kogayonon
{
  class WorldLayer :public Layer
  {
  public:
    explicit WorldLayer(const Shader& shader) :m_shader(shader)
    {
    }

    void drawModels()const;
    void draw()override;
  private:
    Shader m_shader;
  };
}
