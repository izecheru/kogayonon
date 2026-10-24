#include "gui/imgui_windows/entity_properties.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "core/ecs/components/player_input_component.hpp"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include "core/ecs/components/player_component.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/scene/scene_manager.hpp"
#include "core/ecs/components/cubemap_component.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_event_handler.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/utils/font_keys.hpp"
#include "gui/utils/imgui_dragdrop_defines.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "physics/jolt_physics.hpp"
#include "utilities/fonts/materialdesign.hpp"
#include <imgui_stdlib.h>

gui::EntityProperties::EntityProperties( const std::string& name, const EntityPropertiesSpec& spec )
    : ImGuiWindow{ name }
    , m_spec{ spec }
{
}

void gui::EntityProperties::render()
{
    if ( !begin() )
        return;

    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    if ( sceneManager->getEventHandler()->getCurrentEntityId() == entt::null )
    {
        ImGui::PushFont( m_spec.fonts->at( INTER_I ), 18.0f );
        ImGui::Text( "No entity selected..." );
        ImGui::PopFont();
        ImGui::End();
        return;
    }

    contextMenu();

    renderIdentification();
    renderTransform();
    renderModel();
    renderCamera();
    renderRigidbody();
    renderCubemap();

    ImGui::End();
}

void gui::EntityProperties::contextMenu()
{
    ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
    if ( ImGui::BeginCombo( "##", "Add component" ) )
    {
        core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
        core::Scene* scene = sceneManager->getCurrentScene();
        entt::entity currentEntity = sceneManager->getEventHandler()->getCurrentEntityId();
        core::ModelComponent* modelComp = scene->getRegistry()->tryGetComponent<core::ModelComponent>( currentEntity );

        if ( !modelComp )
        {
            if ( ImGui::MenuItem( "Model" ) )
            {
                auto registry = scene->getRegistry();
                registry->addComponent<core::TransformComponent>( currentEntity, core::TransformComponent{} );
                registry->addComponent<core::ModelComponent>( currentEntity,
                                                              core::ModelComponent{
                                                                  .pModel = nullptr,
                                                              } );
            }
        }
        else
        {
            RenderDisabled( ImGui::MenuItem( "Model" ) );
        }

        bool hasCubemap = scene->getRegistry()->hasComponent<core::CubemapComponent>( currentEntity );

        if ( !hasCubemap )
        {
            if ( ImGui::MenuItem( "Add cubemap" ) )
            {
                std::filesystem::path p{ std::filesystem::current_path() / "engine_resources" / "textures" };
                const char* filters[] = { "*.png", "*.jpg" };
                char* result = tinyfd_selectFolderDialog( "Pick folder", p.string().c_str() );

                if ( result )
                {
                    core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
                    resources::Texture* cubemap = assetManager->loadCubemap( result );

                    auto cubemapView = scene->getEnttRegistry().view<core::CubemapComponent>();

                    // on load just disable all other cubemaps
                    cubemapView.each( [&]( const entt::entity& entityId, core::CubemapComponent& cubemapComp ) {
                        if ( cubemapComp.isUsed && currentEntity != entityId )
                            cubemapComp.isUsed = false;
                    } );

                    scene->getRegistry()->addComponent<core::CubemapComponent>(
                        currentEntity, core::CubemapComponent{ .cubemapTexture = cubemap, .isUsed = true } );
                }
            }
        }

        bool hasRigidBody = scene->getRegistry()->hasComponent<core::RigidbodyComponent>( currentEntity );

        if ( !hasRigidBody && modelComp )
        {
            if ( ImGui::MenuItem( "Dynamic rigid body" ) )
            {
                physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

                core::TransformComponent& transform =
                    scene->getRegistry()->getComponent<core::TransformComponent>( currentEntity );

                scene->getRegistry()->addComponent<core::RigidbodyComponent>(
                    currentEntity,
                    core::RigidbodyComponent{
                        .data{ .type = physics::RigidbodyType::Dynamic,
                               .shape = physics::RigidbodyShape::Box,
                               .layer = Layers::NON_MOVING,
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
            }
        }

        if ( !hasRigidBody && modelComp )
        {
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
            }
        }

        ImGui::EndCombo();
    }
    ImGui::PopFont();
}

auto gui::EntityProperties::renderModel() -> void
{
    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    entt::entity currentEntity = sceneManager->getEventHandler()->getCurrentEntityId();

    if ( currentEntity == entt::null )
        return;

    core::Scene* scene = sceneManager->getCurrentScene();
    core::ModelComponent* modelComponent = scene->getRegistry()->tryGetComponent<core::ModelComponent>( currentEntity );

    if ( !modelComponent )
        return;

    gui_utils::renderWithFont( m_spec.fonts->at( INTER_I ), [&]() { ImGui::SeparatorText( "Model component" ); } );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( ICON_MDI ), 12.0f, [&]() {
        if ( ImGui::Button( ICON_MDI_DELETE "Remove component" ) )
        {
            scene->getRegistry()->removeComponent<core::ModelComponent>( currentEntity );
            scene->getRegistry()->removeComponent<core::TransformComponent>( currentEntity );
        }
    } );

    if ( !modelComponent->pModel )
    {
        gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 14.0f, []() {
            ImGui::Text( "You don't have a loaded mesh at the moment, drop a model file here to load it" );
        } );

        if ( ImGui::BeginDragDropTarget() )
        {
            auto payload = ImGui::AcceptDragDropPayload( MODEL_DROP );

            if ( !payload )
            {
                return;
            }

            std::string dropResult{ static_cast<const char*>( payload->Data ) };
            std::filesystem::path p{ dropResult };
            core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
            resources::Model* pModel = assetManager->loadModel( p.stem().string(), p.string() );

            core::Entity ent{ scene->getRegistry(), currentEntity, p.stem().string() };

            ent.removeComponent<core::TransformComponent>();
            ent.removeComponent<core::ModelComponent>();

            ent.addComponent<core::TransformComponent>( core::TransformComponent{} );
            ent.addComponent<core::ModelComponent>( core::ModelComponent{ .modelPath = p.string(), .pModel = pModel } );
        }
    }
    else
    {
        auto modelPath = std::filesystem::path{ modelComponent->pModel->getPath() };
        ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
        ImGui::Text( "Filename: %s", modelPath.stem().string().c_str() );
        ImGui::PopFont();
    }
}

auto gui::EntityProperties::renderCamera() -> void
{
    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    entt::entity currentEntity = sceneManager->getEventHandler()->getCurrentEntityId();

    if ( currentEntity == entt::null )
        return;

    core::Scene* scene = sceneManager->getCurrentScene();
    core::PerspectiveCameraComponent* camera =
        scene->getRegistry()->tryGetComponent<core::PerspectiveCameraComponent>( currentEntity );

    if ( !camera )
        return;

    ImGui::Checkbox( "Is used", &camera->isUsed );

    gui_utils::renderWithFont( m_spec.fonts->at( INTER_I ), []() { ImGui::SeparatorText( "Camera component" ); } );
    // this is if we want to link the values and make them equal
    static bool translationLink{ false };
    static bool centerLink{ false };
    static bool rotationLink{ false };

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Center" ); } );
    ImGui::SameLine();

    // render center here
    if ( !centerLink )
    {
        renderScale( camera->props.changed, camera->props.center );
    }
    else
    {
        RenderDisabled( renderTranslation( camera->props.changed, camera->props.center ) );
        ImGui::SameLine();

        gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
            ImGui::PushItemWidth( 50.0f );
            camera->props.changed |= ImGui::DragFloat( "##transCameraTranslation", &camera->props.center.x, 0.04f );
            ImGui::PopItemWidth();
        } );
        camera->props.center.y = camera->props.center.x;
        camera->props.center.z = camera->props.center.x;
    }

    ImGui::SameLine();
    gui_utils::renderWithSizedFont(
        m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##tLink", &centerLink ); } );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Translation" ); } );
    ImGui::SameLine();

    // render translation here
    if ( !translationLink )
    {
        renderTranslation( camera->props.changed, camera->props.eye );
    }
    else
    {
        RenderDisabled( renderTranslation( camera->props.changed, camera->props.eye ) );
        ImGui::SameLine();

        gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
            ImGui::PushItemWidth( 50.0f );
            camera->props.changed |= ImGui::DragFloat( "##transCameraTranslation", &camera->props.eye.x, 0.04f );
            ImGui::PopItemWidth();
        } );
        camera->props.eye.y = camera->props.eye.x;
        camera->props.eye.z = camera->props.eye.x;
    }

    ImGui::SameLine();
    gui_utils::renderWithSizedFont(
        m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##tLink", &translationLink ); } );

    auto renderCameraProp = [&]( const char* label, bool& changed, float& value ) {
        ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
        changed |= ImGui::DragFloat( label, &value );
        ImGui::PopFont();
    };

    renderCameraProp( "FOV", camera->props.changed, camera->props.fov );
    renderCameraProp( "Near", camera->props.changed, camera->props.nearView );
    renderCameraProp( "Far", camera->props.changed, camera->props.farView );

    camera->updateUbo();
}

auto gui::EntityProperties::renderRigidbody() -> void
{
    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    entt::entity currentEntity = sceneManager->getEventHandler()->getCurrentEntityId();

    if ( currentEntity == entt::null )
        return;

    core::Scene* scene = sceneManager->getCurrentScene();

    core::RigidbodyComponent* rigidBody =
        scene->getRegistry()->tryGetComponent<core::RigidbodyComponent>( currentEntity );

    if ( !rigidBody )
        return;

    gui_utils::renderWithFont( m_spec.fonts->at( INTER_I ), []() { ImGui::SeparatorText( "Rigidbody component" ); } );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
        ImGui::Text( "Shape: %s", physics::rigidbodyShapeStr( rigidBody->data.shape ).c_str() );
        ImGui::Text( "Type: %s", physics::rigidbodyTypeStr( rigidBody->data.type ).c_str() );
    } );
}

auto gui::EntityProperties::renderTransform() -> void
{
    auto sceneManager = core::MainRegistry::getInstance().getSceneManager();
    auto currentEntity = sceneManager->getEventHandler()->getCurrentEntityId();

    if ( currentEntity == entt::null )
        return;

    auto scene = sceneManager->getCurrentScene();

    auto pTransform = scene->getRegistry()->tryGetComponent<core::TransformComponent>( currentEntity );

    if ( !pTransform )
        return;

    gui_utils::renderWithFont( m_spec.fonts->at( INTER_I ), []() { ImGui::SeparatorText( "Transform component" ); } );
    // this is if we want to link the values and make them equal
    static bool translationLink{ false };
    static bool scaleLink{ false };
    static bool rotationLink{ false };

    bool translationChanged{ false };
    bool scaleChanged{ false };
    bool rotationChanged{ false };

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Translation" ); } );
    ImGui::SameLine( ImGui::CalcTextSize( "Translation" ).x );
    // render translation here
    if ( !translationLink )
    {
        renderTranslation( translationChanged, pTransform->translation );
    }
    else
    {
        RenderDisabled( renderTranslation( translationChanged, pTransform->translation ) );
        ImGui::SameLine();

        gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
            ImGui::PushItemWidth( 50.0f );
            translationChanged |= ImGui::DragFloat( "##transTranslation", &pTransform->translation.x, 0.04f );
            ImGui::PopItemWidth();
        } );

        pTransform->translation.y = pTransform->translation.x;
        pTransform->translation.z = pTransform->translation.x;
    }
    ImGui::SameLine();
    gui_utils::renderWithSizedFont(
        m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##tLink", &translationLink ); } );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Scale" ); } );
    ImGui::SameLine( ImGui::CalcTextSize( "Translation" ).x );

    // render scale here
    if ( !scaleLink )
    {
        renderScale( scaleChanged, pTransform->scale );
    }
    else
    {
        RenderDisabled( renderScale( scaleChanged, pTransform->scale ) );
        ImGui::SameLine();

        gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
            ImGui::PushItemWidth( 50.0f );
            translationChanged |= ImGui::DragFloat( "##transScale", &pTransform->scale.x, 0.04f );
            ImGui::PopItemWidth();
        } );

        pTransform->scale.y = pTransform->scale.x;
        pTransform->scale.z = pTransform->scale.x;
    }
    ImGui::SameLine();

    gui_utils::renderWithSizedFont(
        m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##sLink", &scaleLink ); } );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Rotation" ); } );
    ImGui::SameLine( ImGui::CalcTextSize( "Translation" ).x );
    // render scale here
    if ( !rotationLink )
    {
        renderRotation( rotationChanged, pTransform->rotation );
    }
    else
    {
        RenderDisabled( renderRotation( rotationChanged, pTransform->rotation ) );
        ImGui::SameLine();

        gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
            ImGui::PushItemWidth( 50.0f );
            translationChanged |= ImGui::DragFloat( "##transRotation", &pTransform->rotation.x, 0.04f );
            ImGui::PopItemWidth();
        } );

        pTransform->rotation.y = pTransform->rotation.x;
        pTransform->rotation.z = pTransform->rotation.x;
    }
    ImGui::SameLine();

    gui_utils::renderWithSizedFont(
        m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##rLink", &rotationLink ); } );

    if ( translationChanged || scaleChanged || rotationChanged )
    {
        pTransform->computeMatrix();

        auto jolt = core::MainRegistry::getInstance().getJoltPhysics();
        auto pBody = scene->getRegistry()->tryGetComponent<core::RigidbodyComponent>( currentEntity );
        if ( pBody && !jolt->isRunning() )
        {
            // Now set position and rotation for the rigid body
            auto& bodyInterface = jolt->getPhysicsSystem().GetBodyInterface();
            auto quat = pTransform->getOrientation();
            bodyInterface.SetPositionAndRotation(
                pBody->body,
                { pTransform->translation.x, pTransform->translation.y, pTransform->translation.z },
                JPH::Quat{ quat.x, quat.y, quat.z, quat.w },
                pBody->data.activation );
        }
    }
}

auto gui::EntityProperties::renderTranslation( bool& translationChanged, glm::vec3& translation ) -> void
{
    ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
    ImGui::PushItemWidth( 50.0f );

    renderColoredAxis( "X##t", COL_RED_LA );
    ImGui::SameLine();

    translationChanged |= ImGui::DragFloat( "##transCameraX", &translation.x, 0.04f );
    ImGui::SameLine();

    renderColoredAxis( "Y##t", COL_GREEN_LA );
    ImGui::SameLine();

    translationChanged |= ImGui::DragFloat( "##transCameraY", &translation.y, 0.04f );
    ImGui::SameLine();

    renderColoredAxis( "Z##t", COL_BLUE_LA );
    ImGui::SameLine();

    translationChanged |= ImGui::DragFloat( "##transCameraZ", &translation.z, 0.04f );
    ImGui::PopFont();
    ImGui::PopItemWidth();
}

auto gui::EntityProperties::renderScale( bool& scaleChanged, glm::vec3& scale ) -> void
{
    ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
    ImGui::PushItemWidth( 50.0f );

    renderColoredAxis( "X##s", COL_RED_LA );
    ImGui::SameLine();

    scaleChanged = ImGui::DragFloat( "##scaleCameraX", &scale.x, 0.04f );
    ImGui::SameLine();

    renderColoredAxis( "Y##s", COL_GREEN_LA );
    ImGui::SameLine();

    scaleChanged |= ImGui::DragFloat( "##scaleCameraY", &scale.y, 0.04f );
    ImGui::SameLine();

    renderColoredAxis( "Z##s", COL_BLUE_LA );
    ImGui::SameLine();

    scaleChanged |= ImGui::DragFloat( "##scaleCameraZ", &scale.z, 0.04f );

    ImGui::PopFont();
    ImGui::PopItemWidth();
}

auto gui::EntityProperties::renderRotation( bool& rotationChanged, glm::vec3& rotation ) -> void
{
    ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
    ImGui::PushItemWidth( 50.0f );

    renderColoredAxis( "X##r", COL_RED_LA );
    ImGui::SameLine();

    rotationChanged = ImGui::DragFloat( "##rotationX", &rotation.x, 0.04f );
    ImGui::SameLine();

    renderColoredAxis( "Y##r", COL_GREEN_LA );
    ImGui::SameLine();

    rotationChanged |= ImGui::DragFloat( "##rotationY", &rotation.y, 0.04f );
    ImGui::SameLine();

    renderColoredAxis( "Z##r", COL_BLUE_LA );
    ImGui::SameLine();

    rotationChanged |= ImGui::DragFloat( "##rotationZ", &rotation.z, 0.04f );
    ImGui::PopFont();
    ImGui::PopItemWidth();
}

auto gui::EntityProperties::renderIdentification() -> void
{
    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    entt::entity currentEntity = sceneManager->getEventHandler()->getCurrentEntityId();
    core::Scene* scene = sceneManager->getCurrentScene();

    if ( currentEntity == entt::null )
        return;

    core::IdentifierComponent* idComp =
        scene->getRegistry()->tryGetComponent<core::IdentifierComponent>( currentEntity );

    if ( !idComp )
        return;

    ImGui::PushFont( m_spec.fonts->at( INTER_I ) );
    ImGui::SeparatorText( "Entity identification" );
    ImGui::PopFont();

    ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
    ImGui::InputText( "##id", &idComp->name );
    ImGui::Text( "Group: %s", idComp->group.c_str() );
    ImGui::PopFont();
}

auto gui::EntityProperties::renderColoredAxis( const std::string& axis, const ImU32& color ) -> void
{
    ImGui::PushStyleColor( ImGuiCol_Button, color );
    ImGui::PushStyleColor( ImGuiCol_ButtonActive, color );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, color );
    ImGui::PushStyleColor( ImGuiCol_Border, COL_LIGHT_GRAY_LA );
    ImGui::Button( axis.c_str() );
    ImGui::PopStyleColor( 4 );
}

auto gui::EntityProperties::getSpec() -> EntityPropertiesSpec&
{
    return m_spec;
}

auto gui::EntityProperties::renderCubemap() -> void
{
    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    core::SceneEventHandler* sceneHandler = sceneManager->getEventHandler();
    core::Scene* currentScene = sceneManager->getCurrentScene();

    entt::entity currentEntity = sceneHandler->getCurrentEntityId();

    if ( currentEntity == entt::null )
        return;

    core::Entity ent{ currentScene->getRegistry(), currentEntity };

    if ( !ent.hasComponent<core::CubemapComponent>() )
        return;

    core::CubemapComponent& cubemap = ent.getComponent<core::CubemapComponent>();

    std::filesystem::path p{ cubemap.cubemapTexture->path.c_str() };
    ImGui::Text( "Folder: %s", p.stem().string().c_str() );
    if ( ImGui::IsItemHovered() )
    {
        ImGui::BeginTooltip();
        ImGui::Text( "%s", p.string().c_str() );
        ImGui::EndTooltip();
    }

    if ( ImGui::Checkbox( "Is used", &cubemap.isUsed ) )
    {
        if ( cubemap.isUsed )
        {
            core::Scene* currentScene = sceneManager->getCurrentScene();
            auto cubemapView = currentScene->getEnttRegistry().view<core::CubemapComponent>();

            cubemapView.each( [&]( const entt::entity& entityId, core::CubemapComponent& cubemapComp ) {
                if ( currentEntity != entityId && cubemapComp.isUsed )
                {
                    cubemapComp.isUsed = false;
                }
            } );
        }
    }

    if ( ImGui::Button( "Change cubemap" ) )
    {
        cubemap.isUsed = false;

        core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

        auto result = tinyfd_selectFolderDialog( "Pick folder", p.string().c_str() );

        if ( result )
        {
            resources::Texture* cubemapTex = assetManager->loadCubemap( result );

            cubemap.cubemapTexture = cubemapTex;
            cubemap.isUsed = true;
        }
    }
}
