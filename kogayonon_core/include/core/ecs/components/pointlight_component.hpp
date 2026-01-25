#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include "resources/pointlight.hpp"

namespace kogayonon_core
{
struct PointLightComponent
{
	uint32_t pointLightIndex{ 0u };

	static void createLuaBindings( sol::state& lua )
	{
		lua.new_usertype<PointLightComponent>(
			"PointLightComponent",
			"typeId",
			entt::type_hash<PointLightComponent>::value,
			sol::call_constructor,
			sol::factories( []( uint32_t index ) { return PointLightComponent{ .pointLightIndex = index }; } ),
			"pointLightIndex",
			&PointLightComponent::pointLightIndex );
	}
};
} // namespace kogayonon_core