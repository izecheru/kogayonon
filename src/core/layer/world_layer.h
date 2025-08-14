#pragma once
#include "layer.h"
#include "shader/shader.h"

namespace kogayonon
{
  class WorldLayer :public Layer
  {
  public:

    // TODO probably this should not have a shader member variable
    // but instead an array of some sort with all available shaders.... will think abt it
    explicit WorldLayer(const Shader& shader) :m_shader(shader)
    {
    }

    void drawModels()const;
    void draw()override;
  private:
    Shader m_shader;
  };
}
