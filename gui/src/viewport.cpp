#include "gui/imgui_windows/viewport.hpp"
#include "gui/utils/imgui_dragdrop_defines.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include <cmath>
#include "ImOGuizmo.hpp"
#include "SDL3/SDL.h"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/imgui_event.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_event_handler.hpp"
#include "core/scene/scene_manager.hpp"
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "gui/utils/font_keys.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "physics/jolt_physics.hpp"
#include "utilities/fonts/materialdesign.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "utilities/utils/utils.hpp"

gui::Viewport::Viewport( SDL_Window* mainWindow, const std::string& name, const ViewportSpec& spec )
    : ImGuiWindow{ name, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar }
    , m_spec{ spec }
    , m_mainWindow{ mainWindow }
    , m_guizmoMode{ GuizmoMode::Translate }
    , m_guizmoAxisLock{ AxisLock::None }
    , m_guizmoOp{ ImGuizmo::TRANSLATE }
    , m_guizmoEnabled{ true }
    , m_entityMenu{ false }
    , m_viewportDescriptor{ VK_NULL_HANDLE }
    , m_mouseCoords{ 0.0f, 0.0f }
{
    core::EventDispatcher* pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
    pEventDispatcher->addHandler<core::KeyPressedEvent, &Viewport::onKeyPressed>( *this );
}

void gui::Viewport::render()
{
    ImGuiWindowClass viewportClass;
    viewportClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    ImGui::SetNextWindowClass( &viewportClass );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f } );
    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, { 0.0f, 0.0f } );

    if ( !begin() )
        return;

    if ( !m_viewportDescriptor )
    {
        end();
        return;
    }

    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    core::Scene* scene = sceneManager->getCurrentScene();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

    ImGui::SetNextItemAllowOverlap();
    ImGui::Image( m_viewportDescriptor, viewportPanelSize );

    if ( ImGui::BeginDragDropTarget() )
    {
        auto payload = ImGui::AcceptDragDropPayload( MODEL_DROP );
        if ( payload )
        {
            std::string dropResult{ static_cast<const char*>( payload->Data ) };
            std::filesystem::path p{ dropResult };
            core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
            resources::Mesh* pMesh = assetManager->loadMesh( p.stem().string(), p.string() );

            core::Entity ent{ scene->getRegistry(), p.stem().string() };

            ent.removeComponent<core::TransformComponent>();
            ent.removeComponent<core::MeshComponent>();

            ent.addComponent<core::TransformComponent>( core::TransformComponent{} );
            ent.addComponent<core::MeshComponent>( core::MeshComponent{ .pMesh = pMesh } );
        }
        ImGui::EndDragDropTarget();
    }

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    drawToolbar();
    drawEntityMenu();

    auto view = scene->getEnttRegistry().view<core::PerspectiveCameraComponent>();

    // Camera guizmo top right
    view.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComp ) {
        if ( cameraComp.isUsed )
        {
            ImOGuizmo::SetDrawList( ImGui::GetWindowDrawList() );
            ImOGuizmo::SetRect( max.x - 110.0f, min.y + 10.0f, 100.0f );
            gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 11.0f, [&]() {
                if ( ImOGuizmo::DrawGizmo(
                         glm::value_ptr( cameraComp.ubo.view ), glm::value_ptr( cameraComp.ubo.projection ), 0.1f ) )
                {
                }
            } );
        }
    } );

    physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();
    entt::entity currentEntity = sceneManager->getEventHandler()->getCurrentEntityId();

    if ( currentEntity != entt::null && m_guizmoEnabled && !jolt->isRunning() )
    {
        ImGuizmo::Enable( true );
        core::TransformComponent* transformComp =
            scene->getRegistry()->tryGetComponent<core::TransformComponent>( currentEntity );
        if ( transformComp )
        {
            ImGuizmo::SetOrthographic( false );
            ImGuizmo::SetDrawlist( ImGui::GetWindowDrawList() );

            ImGuizmo::SetRect( min.x, min.y, max.x - min.x, max.y - min.y );

            auto view = scene->getEnttRegistry().view<core::PerspectiveCameraComponent>();
            entt::entity cameraEntity = entt::null;
            view.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComp ) {
                if ( cameraComp.isUsed )
                {
                    cameraEntity = entityId;
                }
            } );

            core::PerspectiveCameraComponent& cameraComponent =
                scene->getRegistry()->getComponent<core::PerspectiveCameraComponent>( cameraEntity );
            glm::mat4 projection = cameraComponent.ubo.projection;

            // Unflip the projection
            projection[1][1] *= -1;
            ImGuizmo::Manipulate( glm::value_ptr( cameraComponent.ubo.view ),
                                  glm::value_ptr( projection ),
                                  getGuizmoOp(),
                                  ImGuizmo::WORLD,
                                  glm::value_ptr( transformComp->getMatrix() ) );

            if ( ImGuizmo::IsUsing() )
            {
                ImGuizmo::DecomposeMatrixToComponents( glm::value_ptr( transformComp->getMatrix() ),
                                                       glm::value_ptr( transformComp->translation ),
                                                       glm::value_ptr( transformComp->rotation ),
                                                       glm::value_ptr( transformComp->scale ) );

                // If the entity has a rigid body then update the position and rotation of that too
                core::RigidbodyComponent* joltBody =
                    scene->getRegistry()->tryGetComponent<core::RigidbodyComponent>( currentEntity );

                if ( joltBody )
                {
                    // Now set position and rotation for the rigid body
                    JPH::BodyInterface& bodyInterface = jolt->getPhysicsSystem().GetBodyInterface();
                    glm::quat quat = transformComp->getOrientation();

                    bodyInterface.SetPositionAndRotation(
                        joltBody->body,
                        { transformComp->translation.x, transformComp->translation.y, transformComp->translation.z },
                        JPH::Quat{ quat.x, quat.y, quat.z, quat.w },
                        joltBody->data.activation );
                }
            }
        }
    }

    end();
    ImGui::PopStyleVar( 2 );
}

auto gui::Viewport::setViewport( VkImageView imageView ) -> void
{
    if ( m_viewportDescriptor != VK_NULL_HANDLE )
    {
        ImGui_ImplVulkan_RemoveTexture( m_viewportDescriptor );
    }

    m_viewportDescriptor =
        ImGui_ImplVulkan_AddTexture( m_spec.sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
}

void gui::Viewport::drawToolbar()
{
    auto& style = ImGui::GetStyle();
    physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

    ImGui::SetCursorPos( { 20.0f, 20.0f } );

    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, { 8.0f, 8.0f } );
    ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 10.0f );
    ImGui::PushStyleColor( ImGuiCol_ChildBg, { 0.15f, 0.15f, 0.15f, 0.75f } );

    uint32_t buttonCount = 4u;
    float toolbarWidth = style.WindowPadding.x * 2.0f + ( 14.0f * buttonCount ) +
                         ( style.ItemSpacing.x * buttonCount ) + ( 2.0f * 5.0f );

    ImGui::BeginGroup();
    if ( ImGui::BeginChild( "Toolbar",
                            { toolbarWidth, 70.0f },
                            false,
                            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
    {

        ImGui::PushStyleColor( ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f } );
        ImGui::PushStyleColor( ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 0.0f } );

        ImGui::SetCursorPos( { 5.0f, 5.5f } );
        physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

        if ( ImGui::ImageButton( "##stopButton", m_spec.stopIcon, { 14.0f, 14.0f } ) )
        {
            jolt->stop();
        }
        ImGui::SameLine();

        if ( ImGui::ImageButton( "##startButton", m_spec.playIcon, { 14.0f, 14.0f } ) )
        {
            jolt->start();
        }

        using enum AxisLock;
        auto currentPos = ImGui::GetCursorPos();
        ImGui::SetCursorPos( { currentPos.x + 5.0f, currentPos.y } );
        gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, []() { ImGui::Text( "Axis lock" ); } );
        ImGui::SameLine();
        if ( m_guizmoAxisLock == X_axis )
        {
            gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, []() {
                ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, "X" );
            } );
        }
        else
        {
            gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, []() {
                ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 0.3f }, "X" );
            } );
        }
        ImGui::SameLine();
        if ( m_guizmoAxisLock == Y_axis )
        {
            gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, []() {
                ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, "Y" );
            } );
        }
        else
        {
            gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, []() {
                ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 0.3f }, "Y" );
            } );
        }
        ImGui::SameLine();
        if ( m_guizmoAxisLock == Z_axis )
        {
            gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, []() {
                ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, "Z" );
            } );
        }
        else
        {
            gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, []() {
                ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 0.3f }, "Z" );
            } );
        }

        currentPos = ImGui::GetCursorPos();
        ImGui::SetCursorPos( { currentPos.x + 5.0f, currentPos.y } );
        gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, [&]() {
            if ( jolt->isRunning() )
            {
                ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, "Physics ON" );
            }
            else
            {
                ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 0.3f }, "Physics OFF" );
            }
        } );
    }

    ImGui::PopStyleColor( 2 );

    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar( 2 );
}

void gui::Viewport::drawEntityMenu()
{
    if ( !m_entityMenu )
        return;

    if ( m_mouseCoords.x == 0.0f && m_mouseCoords.y == 0.0f )
    {
        auto mouse = ImGui::GetMousePos();
        if ( m_props->hovered )
        {
            m_mouseCoords = { mouse.x, mouse.y };
        }
        else
        {
            auto size = ImGui::GetWindowSize();
            m_mouseCoords = { m_props->x + ( size.x * 0.5f ) - 130.0f, m_props->y + ( size.y * 0.5f ) - 100.0f };
        }
    }

    ImGui::SetNextWindowPos( { m_mouseCoords.x, m_mouseCoords.y } );

    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f } );
    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, { 10.0f, 10.0f } );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 10.0f );
    ImGui::PushStyleVar( ImGuiStyleVar_PopupRounding, 10.0f );
    ImGui::PushStyleColor( ImGuiCol_ChildBg, { 0.15f, 0.15f, 0.15f, 1.0f } );

    if ( ImGui::Begin( "##quickMenu",
                       nullptr,
                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) )
    {
        // Close the menu if we the mouse does not hover over the window but we detect a click
        if ( !ImGui::IsWindowHovered( ImGuiHoveredFlags_RootAndChildWindows ) &&
             ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
        {
            m_entityMenu = false;
            m_mouseCoords = { 0.0f, 0.0f };
        }

        core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
        core::Scene* scene = sceneManager->getCurrentScene();
        core::EventDispatcher* pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
        core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

        ImGui::PushFont( m_spec.fonts->at( INTER ), 14.0f );

        if ( ImGui::BeginMenu( "Add object" ) )
        {
            std::string filename{ "" };
            bool selected{ false };

            if ( ImGui::MenuItem( "Cone" ) )
            {
                filename = "default_cone";
                selected = true;
            }

            if ( ImGui::MenuItem( "Cube" ) )
            {
                filename = "cube";
                selected = true;
            }

            if ( ImGui::MenuItem( "Sphere" ) )
            {
                filename = "sphere";
                selected = true;
            }

            if ( ImGui::MenuItem( "Ico Sphere" ) )
            {
                filename = "ico_sphere";
                selected = true;
            }

            if ( ImGui::MenuItem( "Cylinder" ) )
            {
                filename = "cylinder";
                selected = true;
            }

            if ( ImGui::MenuItem( "Torus" ) )
            {
                filename = "torus";
                selected = true;
            }

            if ( ImGui::MenuItem( "Plane" ) )
            {
                filename = "plane";
                selected = true;
            }

            if ( selected )
            {
                core::Entity ent{ scene->getRegistry(), filename };

                filename += ".gltf";
                std::filesystem::path p{ std::filesystem::absolute( "." ) / "engine_resources" / "models" / filename };

                ent.addComponent<core::TransformComponent>( core::TransformComponent{} );
                ent.addComponent<core::MeshComponent>(
                    core::MeshComponent{ .pMesh = assetManager->loadMesh( "test", p.string() ) } );

                pEventDispatcher->dispatchEvent<core::SelectEntityEvent>(
                    core::SelectEntityEvent{ ent.getEntityId(), core::SelectEntityEventSource::Viewport_Window } );

                m_entityMenu = false;
                m_mouseCoords = { 0.0f, 0.0f };
            }
            ImGui::EndMenu();
        }

        entt::entity currentEntity = sceneManager->getEventHandler()->getCurrentEntityId();
        if ( currentEntity != entt::null )
        {
            if ( ImGui::BeginMenu( "Add component" ) )
            {
                if ( !scene->getRegistry()->hasComponent<core::RigidbodyComponent>( currentEntity ) )
                {
                    if ( ImGui::MenuItem( "Dynamic rigid body" ) )
                    {
                        auto jolt = core::MainRegistry::getInstance().getJoltPhysics();
                        auto& transform = scene->getRegistry()->getComponent<core::TransformComponent>( currentEntity );
                        scene->getRegistry()->addComponent<core::RigidbodyComponent>(
                            currentEntity,
                            core::RigidbodyComponent{
                                .data{ .type = physics::RigidbodyType::Dynamic,
                                       .shape = physics::RigidbodyShape::Box,
                                       .layer = Layers::MOVING,
                                       .motionType = JPH::EMotionType::Dynamic,
                                       .activation = JPH::EActivation::Activate },
                                .body = jolt->createRigidBody(
                                    physics::RigidbodyType::Dynamic,
                                    physics::RigidbodyShape::Box,
                                    { transform.translation.x, transform.translation.y, transform.translation.z },
                                    { transform.scale.x,
                                      transform.scale.y,
                                      transform.scale
                                          .z }, // TODO(kogayonon) detemrine size somehow, with a bounding box i guess
                                    transform.getOrientation() ) } );

                        m_entityMenu = false;
                        m_mouseCoords = { 0.0f, 0.0f };
                    }

                    if ( ImGui::MenuItem( "Static rigid body (terrain)" ) )
                    {
                        auto jolt = core::MainRegistry::getInstance().getJoltPhysics();
                        core::TransformComponent& transform =
                            scene->getRegistry()->getComponent<core::TransformComponent>( currentEntity );

                        core::MeshComponent& meshComponent =
                            scene->getRegistry()->getComponent<core::MeshComponent>( currentEntity );

                        utilities::TaskManager* taskManager = core::MainRegistry::getInstance().getTaskManager();

                        auto callback = taskManager->addTask( [=, meshPtr = meshComponent.pMesh]() {
                            JPH::VertexList vertices{};
                            JPH::IndexedTriangleList indices{};

                            auto& vert = meshPtr->getVertices();
                            auto& ind = meshPtr->getIndices();

                            vertices.reserve( vert.size() );
                            indices.reserve( ind.size() / 3 );

                            for ( const resources::Vertex& v : vert )
                            {
                                vertices.push_back( { v.translation.x, v.translation.y, v.translation.z } );
                            }

                            for ( size_t i = 0; i < ind.size(); i += 3 )
                            {
                                indices.push_back( { ind[i], ind[i + 1], ind[i + 2] } );
                            }

                            {
                                std::lock_guard lock{ scene->getRegistryMutex() };
                                scene->getRegistry()->addComponent<core::RigidbodyComponent>(
                                    currentEntity,
                                    core::RigidbodyComponent{
                                        .data = { .type = physics::RigidbodyType::Static },
                                        .body = jolt->createRigidTerrainBody(
                                            vertices,
                                            indices,
                                            { transform.translation.x,
                                              transform.translation.y,
                                              transform.translation.z },
                                            { transform.scale.x,
                                              transform.scale.y,
                                              transform.scale.z }, // TODO(kogayonon) detemrine size somehow, with a
                                                                   // bounding box i guess
                                            transform.getOrientation() ) } );

                                KINFO( "rigid body created" );
                            }
                        } );
                        taskManager->addTaskSetToPipe( callback );

                        m_entityMenu = false;
                        m_mouseCoords = { 0.0f, 0.0f };
                    }

                    if ( ImGui::MenuItem( "Static rigid body" ) )
                    {
                        auto jolt = core::MainRegistry::getInstance().getJoltPhysics();
                        auto& transform = scene->getRegistry()->getComponent<core::TransformComponent>( currentEntity );
                        scene->getRegistry()->addComponent<core::RigidbodyComponent>(
                            currentEntity,
                            core::RigidbodyComponent{
                                .data{ .type = physics::RigidbodyType::Static,
                                       .shape = physics::RigidbodyShape::Box,
                                       .layer = Layers::NON_MOVING,
                                       .motionType = JPH::EMotionType::Static,
                                       .activation = JPH::EActivation::DontActivate },
                                .body = jolt->createRigidBody(
                                    physics::RigidbodyType::Static,
                                    physics::RigidbodyShape::Box,
                                    { transform.translation.x, transform.translation.y, transform.translation.z },
                                    { transform.scale.x,
                                      transform.scale.y,
                                      transform.scale
                                          .z }, // TODO(kogayonon) detemrine size somehow, with a bounding box i guess
                                    transform.getOrientation() ) } );

                        m_entityMenu = false;
                        m_mouseCoords = { 0.0f, 0.0f };
                    }
                }
                else
                {
                    RenderDisabled( ImGui::MenuItem( "Dynamic rigid body" ) );
                    RenderDisabled( ImGui::MenuItem( "Static rigid body" ) );
                }
                ImGui::EndMenu();
            }
        }
        else
        {
            RenderDisabled( ImGui::MenuItem( "Add component" ) );
        }

        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar( 4 );
}

auto gui::Viewport::getGuizmoOp() -> ImGuizmo::OPERATION
{
    using enum GuizmoMode;
    using enum AxisLock;
    switch ( m_guizmoMode )
    {
    case Scale: {
        switch ( m_guizmoAxisLock )
        {
        case None:
            return ImGuizmo::SCALE;
        case X_axis:
            return ImGuizmo::SCALE_X;
        case Y_axis:
            return ImGuizmo::SCALE_Y;
        case Z_axis:
            return ImGuizmo::SCALE_Z;
        }
        break;
    }

    case Rotate: {
        switch ( m_guizmoAxisLock )
        {
        case None:
            return ImGuizmo::ROTATE;
        case X_axis:
            return ImGuizmo::ROTATE_X;
        case Y_axis:
            return ImGuizmo::ROTATE_Y;
        case Z_axis:
            return ImGuizmo::ROTATE_Z;
        }
        break;
    }

    case Translate: {
        switch ( m_guizmoAxisLock )
        {
        case None:
            return ImGuizmo::TRANSLATE;
        case X_axis:
            return ImGuizmo::TRANSLATE_X;
        case Y_axis:
            return ImGuizmo::TRANSLATE_Y;
        case Z_axis:
            return ImGuizmo::TRANSLATE_Z;
        }
        break;
    }
    }

    return ImGuizmo::TRANSLATE;
}

void gui::Viewport::onKeyPressed( const core::KeyPressedEvent& e )
{
    auto pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();

    // Deactivate the Shift + A menu if Escape is pressed
    if ( e.getKeyScanCode() == KeyScanCode::Escape && m_entityMenu )
    {
        m_entityMenu = false;
    }

    // Enable/ Disable Guizmo
    if ( KeyboardState::getKeyCombinationState( { KeyScanCode::LeftShift, KeyScanCode::G } ) )
    {
        m_guizmoEnabled = !m_guizmoEnabled;
    }

    // Open the Shift + A quick menu
    if ( KeyboardState::getKeyCombinationState( { KeyScanCode::LeftShift, KeyScanCode::A } ) )
    {
        if ( m_entityMenu )
        {
            m_mouseCoords = { 0.0f, 0.0f }; // Reset mouse coords on close
        }

        m_entityMenu = !m_entityMenu;
    }

    if ( KeyboardState::getKeyState( KeyScanCode::Escape ) )
    {
        pEventDispatcher->dispatchEvent<core::SelectEntityEvent>( core::SelectEntityEvent{} );
    }

    // If guizmo is enabled we also have some hotkeys for it like axis lock
    if ( !m_guizmoEnabled )
    {
        return;
    }

    using enum GuizmoMode;
    using enum AxisLock;

    const bool shift = e.getKeyModifier() == KeyScanCode::LeftShift || e.getKeyModifier() == KeyScanCode::RightShift;

    if ( !shift )
    {
        return;
    }

    if ( KeyboardState::getKeyCombinationState( { KeyScanCode::LeftShift, KeyScanCode::T } ) )
    {
        m_guizmoMode = Translate;
    }
    else if ( KeyboardState::getKeyCombinationState( { KeyScanCode::LeftShift, KeyScanCode::R } ) )
    {
        m_guizmoMode = Rotate;
    }
    else if ( KeyboardState::getKeyCombinationState( { KeyScanCode::LeftShift, KeyScanCode::S } ) )
    {
        m_guizmoMode = Scale;
    }
}
