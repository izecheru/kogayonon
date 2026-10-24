#include "core/systems/player_input_system.hpp"
#include "utilities/input/mouse_state.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "core/input/key_map_manager.hpp"
#include "core/ecs/registry.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/player_component.hpp"
#include "core/ecs/components/player_input_component.hpp"
#include "physics/jolt_physics.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/event_dispatcher.hpp"

core::PlayerInputSystem::~PlayerInputSystem()
{
}

core::PlayerInputSystem::PlayerInputSystem( EventDispatcher* dispatcher, Registry* registry )
    : m_keyMapManager{ std::make_unique<KeyMapManager>( dispatcher ) }
    , m_registry{ registry }
{
    auto playerView = m_registry->getRegistry()
                          .view<core::PerspectiveCameraComponent, core::PlayerComponent, core::PlayerInputComponent>();
    entt::entity playerId{ entt::null };

    uint32_t count{ 0u };
    playerView.each( [&]( const entt::entity& entityId,
                          core::PerspectiveCameraComponent&,
                          core::PlayerComponent&,
                          core::PlayerInputComponent& ) {
        playerId = entityId;
        ++count;
    } );

    if ( count > 1 )
        throw std::runtime_error( "Somehow you have more than 1 player" );

    if ( playerId == entt::null )
    {
        KERROR( "Input system could not register bindings and actions" );
        return;
    }

    core::PlayerInputComponent* inputComponent = m_registry->tryGetComponent<core::PlayerInputComponent>( playerId );

    core::PerspectiveCameraComponent* cameraComponent =
        m_registry->tryGetComponent<core::PerspectiveCameraComponent>( playerId );

    if ( !inputComponent || !cameraComponent )
        throw std::runtime_error( "Player does not posses important components!!!" );

    m_keyMapManager->registerAction( "player", Sprint, [this, inputComponent]() {
        if ( inputComponent->sprint )
        {
            return;
        }

        inputComponent->sprint = true;
    } );

    m_keyMapManager->bindAction( "player", utilities::KeyScanCode::LeftShift, Sprint );

    m_keyMapManager->registerAction( "player", Jump, [this, inputComponent]() {
        if ( inputComponent->jump )
        {
            return;
        }

        inputComponent->jump = true;
    } );

    m_keyMapManager->bindAction( "player", utilities::KeyScanCode::Space, EAction::Jump );

    m_keyMapManager->registerAction( "player", MoveBack, [this, inputComponent, cameraComponent]() {
        inputComponent->move -= cameraComponent->props.forward;
    } );

    m_keyMapManager->bindAction( "player", utilities::KeyScanCode::S, EAction::MoveBack );

    m_keyMapManager->registerAction( "player", MoveForward, [this, inputComponent, cameraComponent]() {
        inputComponent->move += cameraComponent->props.forward;
    } );

    m_keyMapManager->bindAction( "player", utilities::KeyScanCode::W, EAction::MoveForward );

    m_keyMapManager->registerAction( "player", MoveLeft, [this, inputComponent, cameraComponent]() {
        inputComponent->move += cameraComponent->props.right;
    } );

    m_keyMapManager->bindAction( "player", utilities::KeyScanCode::A, EAction::MoveLeft );

    m_keyMapManager->registerAction( "player", MoveRight, [this, inputComponent, cameraComponent]() {
        inputComponent->move -= cameraComponent->props.right;
    } );

    m_keyMapManager->bindAction( "player", utilities::KeyScanCode::D, EAction::MoveRight );
}

auto core::PlayerInputSystem::update( entt::registry& reg, float delta ) -> void
{
    physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

    m_keyMapManager->updateGroup( "player" );

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

    if ( !cameraComponent->isUsed )
        cameraComponent->isUsed = true;

    JPH::Vec3 currentVelocity = playerComponent->character->GetLinearVelocity();

    utilities::TimeTracker* timeTracker = core::MainRegistry::getInstance().getTimeTracker();

    float yawRad = glm::radians( cameraComponent->props.yaw );

    glm::vec3 forward{ cos( yawRad ), 0.0f, sin( yawRad ) };
    glm::vec3 right{ -sin( yawRad ), 0.0f, cos( yawRad ) };

    double speed{ 6.5f };

    core::PlayerInputComponent* inputComponent = m_registry->tryGetComponent<core::PlayerInputComponent>( playerId );

    if ( !inputComponent )
        return;

    constexpr float jumpSpeed{ 8.0f };
    const bool onGround = playerComponent->isOnGround();

    // sprint only on ground
    if ( inputComponent->sprint && onGround )
        speed += 10.0f;

    inputComponent->sprint = false;

    inputComponent->move.x *= speed;
    inputComponent->move.z *= speed;

    float vy = currentVelocity.GetY();
    const bool jumpRequested = inputComponent->jump;

    inputComponent->jump = false;

    if ( jumpRequested && onGround )
    {
        vy = jumpSpeed;
    }
    else if ( !onGround )
    {

        float g = -jolt->getPhysicsSystem().GetGravity().GetY();

        if ( vy < 0.0f )
            g *= 1.5f;

        vy -= g * delta;
    }
    else
    {
        vy = 0.0f;
    }

    JPH::Vec3 desiredVelocity{ inputComponent->move.x, vy, inputComponent->move.z };

    if ( onGround )
    {
        desiredVelocity += jolt->getPhysicsSystem().GetGravity() * delta;
    }

    playerComponent->character->SetLinearVelocity( desiredVelocity );
    inputComponent->move = { 0.0f, 0.0f, 0.0f };

    JPH::RVec3 newPos = playerComponent->character->GetPosition();

    // update camera to match character
    cameraComponent->props.eye = { newPos.GetX(), newPos.GetY(), newPos.GetZ() };

    double sensitivity{ 0.2f };
    glm::vec2 accumulatedRel = utilities::MouseState::getRel();
    cameraComponent->props.yaw += accumulatedRel.x * sensitivity;
    cameraComponent->props.pitch -= accumulatedRel.y * sensitivity;
    utilities::MouseState::resetRel();

    // renderer will update this later
    cameraComponent->props.changed = true;
}
