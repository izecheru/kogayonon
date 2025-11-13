#pragma once
#include <cinttypes>
#include "rendering/uniformbuffer.hpp"
#include "resources/light_types.hpp"

namespace kogayonon_rendering
{
struct LightCount
{
  int numPointLights{ 0 };
  int numDirectionalLigths{ 0 };
  int numSpotLigths{ 0 };
  int pad;
};

class LightCountUniformbuffer : public Uniformbuffer
{
public:
  LightCountUniformbuffer() = default;
  ~LightCountUniformbuffer() = default;

  void incrementLightCount( const kogayonon_resources::LightType& type );
  void decrementLightCount( const kogayonon_resources::LightType& type );
  uint32_t getLightCount( const kogayonon_resources::LightType& type ) const;

  void initialize( uint32_t bindingIndex ) override;
  void destroy() override;
  void update();
  void bind() override;
  void unbind() override;

private:
  uint32_t m_ubo{ 0 };
  uint32_t m_bindingIndex{ 0 };

  LightCount m_count{};
};
} // namespace kogayonon_rendering