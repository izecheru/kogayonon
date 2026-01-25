#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace kogayonon_core
{
struct DirectionalLightComponent
{
	int directionalLightIndex{ 0 };
	float nearPlane{ 0.1f };
	float farPlane{ 300.0f };
	float orthoSize{ 70.0f };
	float positionFactor{ 20.0f };

	static void createLuaBindings( sol::state& lua )
	{
		lua.new_usertype<DirectionalLightComponent>(
			"DirectionalLightComponent",
			"typeId",
			entt::type_hash<DirectionalLightComponent>::value,
			sol::call_constructor,
			sol::factories( []() { return DirectionalLightComponent{}; },
							[]( float near, float far, float orthoSize, float posFactor ) {
								return DirectionalLightComponent{ .nearPlane = near,
																  .farPlane = far,
																  .orthoSize = orthoSize,
																  .positionFactor = posFactor };
							} ),
			"directionalLightIndex",
			&DirectionalLightComponent::directionalLightIndex,
			"nearPlane",
			&DirectionalLightComponent::nearPlane,
			"farPlane",
			&DirectionalLightComponent::farPlane,
			"orthoSize",
			&DirectionalLightComponent::orthoSize,
			"positionFactor",
			&DirectionalLightComponent::positionFactor );
	}
};
} // namespace kogayonon_core