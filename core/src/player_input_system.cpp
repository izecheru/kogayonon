#include "core/systems/player_input_system.hpp"
#include "input/mouse_state.hpp"
#include "time_tracker/time_tracker.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "core/ecs/registry.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/player_component.hpp"
#include "core/ecs/components/player_input_component.hpp"
#include "physics/jolt_physics.hpp"
#include "core/event/event_dispatcher.hpp"

core::PlayerInputSystem::PlayerInputSystem( EventDispatcher* dispatcher )
{
    // dispatcher->addHandler<core::MouseMovedEvent, &PlayerInputSystem::onMouseMove>( *this );
}

auto core::PlayerInputSystem::disableEditorCamera( entt::registry& reg ) -> void
{
    physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

    const bool joltRunning = jolt->isRunning();

    // get the camera entities without the one that also has a player
    auto cameraView = reg.view<core::PerspectiveCameraComponent>( entt::exclude<core::PlayerComponent> );

    cameraView.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComponent ) {
        // if we have enabled physics and have an active player entity, disable all other cameras
        if ( joltRunning && reg.view<core::PlayerComponent>().size() != 0 )
        {
            if ( cameraComponent.isUsed )
                cameraComponent.isUsed = false;
        }
        else
        {
            if ( !cameraComponent.isUsed )
                cameraComponent.isUsed = true;
        }
    } );
}

auto core::PlayerInputSystem::update( entt::registry& reg, float delta ) -> void
{
    physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

    auto playerView = reg.view<core::PerspectiveCameraComponent, core::PlayerComponent>();
    entt::entity playerId{ entt::null };

    uint32_t count{ 0u };
    playerView.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent&, core::PlayerComponent& ) {
        playerId = entityId;
        ++count;
    } );

    if ( count > 1 )
        throw std::runtime_error( "Somehow you have more than 1 player" );

    if ( playerId == entt::null )
        return;

    core::PerspectiveCameraComponent* cameraComponent = reg.try_get<core::PerspectiveCameraComponent>( playerId );
    core::PlayerComponent* playerComponent = reg.try_get<core::PlayerComponent>( playerId );

    if ( !cameraComponent || !playerComponent )
        return;

    disableEditorCamera( reg );

    if ( !cameraComponent->isUsed )
        cameraComponent->isUsed = true;

    JPH::Vec3 currentVelocity = playerComponent->character->GetLinearVelocity();

    utilities::TimeTracker* timeTracker = core::MainRegistry::getInstance().getTimeTracker();

    float yawRad = glm::radians( cameraComponent->props.yaw );

    glm::vec3 forward{ cos( yawRad ), 0.0f, sin( yawRad ) };
    glm::vec3 right{ -sin( yawRad ), 0.0f, cos( yawRad ) };

    glm::vec3 moveDir{ 0.0f };

    using utilities::KeyScanCode;

    if ( utilities::KeyboardState::getKeyState( KeyScanCode::W ) )
        moveDir += forward;

    if ( utilities::KeyboardState::getKeyState( KeyScanCode::S ) )
        moveDir -= forward;

    if ( utilities::KeyboardState::getKeyState( KeyScanCode::D ) )
        moveDir += right;

    if ( utilities::KeyboardState::getKeyState( KeyScanCode::A ) )
        moveDir -= right;

    constexpr float jumpSpeed{ 3.0f };
    const float gravity{ jolt->getPhysicsSystem().GetGravity().GetY() * -1.0f };
    constexpr float fallMultiplier{ 1.9f };

    bool onGround = playerComponent->isOnGround();
    bool spaceDown = utilities::KeyboardState::getKeyState( KeyScanCode::Space );

    float vy = currentVelocity.GetY();

    if ( onGround )
    {
        vy = 0.0f;

        if ( spaceDown )
            vy = jumpSpeed;
    }
    else
    {
        float g = gravity;

        // as long as we still fall, increase the gravity
        // the higher we go, the faster we hit the ground
        if ( vy < 0.0f )
            g *= fallMultiplier;

        vy -= g * delta;
    }

    if ( glm::length( moveDir ) > 0.001f )
        moveDir = glm::normalize( moveDir );

    double speed{ 6.5f };

    const bool leftShiftDown = utilities::KeyboardState::getKeyState( KeyScanCode::LeftShift );

    if ( leftShiftDown && onGround )
        speed = 10.0f;

    moveDir.x *= speed;
    moveDir.z *= speed;

    JPH::Vec3 desiredVelocity{ moveDir.x, vy, moveDir.z };

    if ( onGround )
    {
        desiredVelocity += jolt->getPhysicsSystem().GetGravity() * delta;
    }

    playerComponent->character->SetLinearVelocity( desiredVelocity );

    JPH::RVec3 newPos = playerComponent->character->GetPosition();

    // update camera to match character
    cameraComponent->props.eye = { newPos.GetX(), newPos.GetY(), newPos.GetZ() };

    double sensitivity{ 0.2f };
    glm::vec2 accumulatedRel = utilities::MouseState::readAccumulated();
    cameraComponent->props.yaw += accumulatedRel.x * sensitivity;
    cameraComponent->props.pitch -= accumulatedRel.y * sensitivity;

    // renderer will update this later
    cameraComponent->props.changed = true;
}
