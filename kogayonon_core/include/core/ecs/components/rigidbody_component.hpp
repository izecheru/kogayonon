#pragma once
#include <physx/PxRigidDynamic.h>
#include <physx/PxRigidStatic.h>
#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace kogayonon_core
{
struct DynamicRigidbodyComponent
{
	physx::PxRigidDynamic* pBody{ nullptr };

	static void createLuaBindings( sol::state& lua )
	{
		lua.new_usertype<DynamicRigidbodyComponent>( "DynamicRigidBodyComponent",
													 "typeId",
													 entt::type_hash<DynamicRigidbodyComponent>::value,
													 sol::call_constructor,
													 sol::factories( []( physx::PxRigidDynamic* pBody ) {
														 return DynamicRigidbodyComponent{ .pBody = pBody };
													 } ),
													 "pBody",
													 &DynamicRigidbodyComponent::pBody );
	}
};

struct StaticRigidbodyComponent
{
	physx::PxRigidStatic* pBody{ nullptr };

	static void createLuaBindings( sol::state& lua )
	{
		lua.new_usertype<StaticRigidbodyComponent>(
			"StaticRigidbodyComponent",
			"typeId",
			entt::type_hash<StaticRigidbodyComponent>::value,
			sol::call_constructor,
			sol::factories( []( physx::PxRigidStatic* pBody ) { return StaticRigidbodyComponent{ .pBody = pBody }; } ),
			"pBody",
			&StaticRigidbodyComponent::pBody );
	}
};
} // namespace kogayonon_core