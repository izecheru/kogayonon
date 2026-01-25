#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include "utilities/math/math.hpp"

namespace kogayonon_core
{
struct TransformComponent
{
	glm::vec3 translation{ 0.0f };
	glm::vec3 rotation{ 0.0f };
	glm::vec3 scale{ 1.0f };

	static void createLuaBindings( sol::state& lua )
	{
		lua.new_usertype<TransformComponent>(
			"TransformComponent",
			"typeId",
			entt::type_hash<TransformComponent>::value,
			sol::call_constructor,

			sol::factories(
				[]( glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale ) {
					return TransformComponent{ .translation = translation, .rotation = rotation, .scale = scale };
				},
				[]() { return TransformComponent{}; } ),

			"translation",
			&TransformComponent::translation,
			"rotation",
			&TransformComponent::rotation,
			"scale",
			&TransformComponent::scale );
	}
};
} // namespace kogayonon_core