#pragma once
#include <cinttypes>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "resources/directional_light.hpp"
#include "resources/light_types.hpp"
#include "resources/pointlight.hpp"
#include "resources/spotlight.hpp"
#include "shader_storagebuffer.hpp"

namespace kogayonon_rendering
{
using point_lights = std::vector<kogayonon_resources::PointLight>;
using directional_lights = std::vector<kogayonon_resources::DirectionalLight>;
using spot_lights = std::vector<kogayonon_resources::SpotLight>;

enum class SSBOType
{
	Point = 0,
	Directional = 1,
	Spot = 2
};

class LightShaderStoragebuffer : public IShaderStorageBuffer
{
  public:
	struct SSBO
	{
		uint32_t id{ 0 };
		uint32_t bindingIndex{ 0 };
	};

	LightShaderStoragebuffer() = default;
	~LightShaderStoragebuffer() = default;

	void unbind() override;
	void bind( uint32_t index ) override;
	void bind() override;

	void initialize() override;
	void initStorageBuffer( const kogayonon_resources::LightType& type, SSBO& ssbo );
	void destroy( uint32_t index ) override;
	void destroy() override;
	void update( uint32_t index ) override;

	void update() override;

	auto addLight( const kogayonon_resources::LightType& type ) -> uint32_t;
	void removeLight( const kogayonon_resources::LightType& type, uint32_t index );

	auto getPointLights() -> point_lights&;
	auto getDirectionalLights() -> directional_lights&;
	auto getSpotLights() -> spot_lights&;

  private:
	std::vector<SSBO> m_ssbos;

	point_lights m_pointLights;
	directional_lights m_directionalLights;
	spot_lights m_spotLights;
};
} // namespace kogayonon_rendering