#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace kogayonon_core
{
struct OutlineComponent
{
	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	static void createLuaBindings( sol::state& lua )
	{
		lua.new_usertype<OutlineComponent>( "OutlineComponent",
											"typeId",
											entt::type_hash<OutlineComponent>::value,

											sol::call_constructor,
											sol::factories( []() { return OutlineComponent{}; },
															[]( float r, float g, float b, float a ) {
																glm::vec4 color{ r, g, b, a };
																return OutlineComponent{ .color = color };
															} ),
											"color",
											&OutlineComponent::color );
	}
};
} // namespace kogayonon_core