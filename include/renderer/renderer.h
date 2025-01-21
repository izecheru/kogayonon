#pragma once
#include <cstdint>
#include <ext/matrix_float4x4.hpp>

namespace kogayonon
{

  class Renderer
  {
  public:
    static void init();
    static void shutdown();

    static void onWindowResize(uint32_t width, uint32_t height);

    static void beginScene();
    static void endScene();

  private:
    struct SceneData
    {
      glm::mat4 view_projection_matrix;
    };

    static SceneData s_scene_data;
  };

}
