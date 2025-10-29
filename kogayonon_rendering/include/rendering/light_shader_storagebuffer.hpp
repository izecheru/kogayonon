#pragma once
#include <cinttypes>
#include <memory>
#include <vector>
#include "resources/pointlight.hpp"
#include "shader_storagebuffer.hpp"

namespace kogayonon_rendering
{
class LightShaderStoragebuffer : public ShaderStoragebuffer
{
public:
  LightShaderStoragebuffer() = default;
  ~LightShaderStoragebuffer() = default;

  void bind() override;
  void unbind() override;

  void initialize( uint32_t bindingIndex ) override;
  void destroy() override;
  void update() override;

  std::vector<kogayonon_resources::PointLight>& getPointLights();
  int addPointLight();

private:
  uint32_t m_ssbo{ 0 };
  uint32_t m_bindingIndex{ 0 };

  std::vector<kogayonon_resources::PointLight> m_pointLights;
};
} // namespace kogayonon_rendering