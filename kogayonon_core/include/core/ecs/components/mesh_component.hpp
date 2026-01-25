#pragma once
#include <entt/entt.hpp>
#include <cinttypes>
#include <sol/sol.hpp>
#include <vector>
#include "resources/mesh.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_core
{

/**
 * @brief An object has one or many meshes, wether we treat it as a single entity or replicate the submesh hierarchy
 * depends on the user desire to do so
 */
struct MeshComponent
{
	kogayonon_resources::Mesh* pMesh;
	bool staticMesh{ false };
	bool loaded{ false };

	static void createLuaBindings( sol::state& lua )
	{
		lua.new_usertype<MeshComponent>(
			"MeshComponent",
			"typeId",
			entt::type_hash<MeshComponent>::value,
			sol::call_constructor,
			sol::factories( []() { return MeshComponent{ .pMesh = nullptr, .staticMesh = false, .loaded = false }; },
							[]( kogayonon_resources::Mesh* pMesh, bool staticMesh, bool loaded ) {
								return MeshComponent{ .pMesh = pMesh, .staticMesh = staticMesh, .loaded = loaded };
							} ),
			"pMesh",
			&MeshComponent::pMesh,
			"staticMesh",
			&MeshComponent::staticMesh,
			"loaded",
			&MeshComponent::loaded );
	}
};
} // namespace kogayonon_core