#pragma once
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"

namespace core
{
struct PlayerComponent
{
    JPH::CharacterVirtualSettings settings{};
    JPH::CharacterVirtual* character{ nullptr };

    auto isOnGround() -> bool
    {
        return character->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
    }
};
} // namespace core