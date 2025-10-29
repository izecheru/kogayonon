#pragma once
#include <cinttypes>
#include "rendering/uniformbuffer.hpp"

namespace kogayonon_rendering
{
struct LightCount
{
  int numPointLights{ 0 };
  int numDirectionalLigths{ 0 };
  int pad;
};

class LightCountUniformbuffer : public Uniformbuffer
{
public:
  LightCountUniformbuffer() = default;
  ~LightCountUniformbuffer() = default;

  void incrementPointLights();
  void incrementDirectionalLights();

  void decrementPointLights();
  void decrementDirectionalLights();

  void initialize( uint32_t bindingIndex ) override;
  void destroy() override;
  void update();
  void bind() override;
  void unbind() override;

private:
  uint32_t m_ubo{ 0 };
  LightCount m_count;
  uint32_t m_bindingIndex{ 0 };
};
} // namespace kogayonon_rendering