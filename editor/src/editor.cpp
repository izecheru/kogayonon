#include "editor/editor.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <rapidjson/istreamwrapper.h>
#include <glm/gtc/type_ptr.hpp>
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/app_event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "core/systems/scene_rendering_system.hpp"
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "physics/jolt_physics.hpp"
#include "renderer/vulkan_renderer.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"
#include "utilities/config_manager/config_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "utilities/utils/utils.hpp"
#include "window/window.hpp"

editor::Editor::Editor()
{
    utilities::EditorConfigManager::initConfig();

    KeyboardState::initState();

    init();

    // create the editor dir
    std::filesystem::path editorDir = std::filesystem::current_path() / "editor";
    if ( !std::filesystem::exists( editorDir ) )
    {
        std::filesystem::create_directory( editorDir );
    }

    // scenes dir
    std::filesystem::path scenesDir = std::filesystem::current_path() / "editor" / "scenes";
    if ( !std::filesystem::exists( scenesDir ) )
    {
        std::filesystem::create_directory( scenesDir );
    }
}

editor::Editor::~Editor()
{
}

void editor::Editor::cleanup() const
{
    auto vkCtx = core::MainRegistry::getInstance().getVulkanContext();
    vkCtx->device->waitIdle();
    vkCtx->device->destroyDescriptorPool( m_globalDescriptorPool );
}

void editor::Editor::pollEvents()
{
    const auto& pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();

    SDL_Event e;
    while ( SDL_PollEvent( &e ) )
    {
        ImGui_ImplSDL3_ProcessEvent( &e );
        switch ( e.type )
        {
        case SDL_EVENT_WINDOW_RESIZED: {
            int newWidth = e.window.data1;
            int newHeight = e.window.data2;
            KINFO( "Resized wind" );
            pEventDispatcher->dispatchEvent( core::WindowResizeEvent{ newWidth, newHeight } );
            break;
        }
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            pEventDispatcher->dispatchEvent( core::WindowCloseEvent{} );
            m_running = false;
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            if ( e.key.repeat )
                break;

            KeyboardState::updateState();
            KeyScanCode scanCode = static_cast<KeyScanCode>( e.key.key );

            core::KeyPressedEvent keyPressEvent{ scanCode, KeyScanCode::None, 0 };

            if ( KeyboardState::getKeyState( KeyScanCode::LeftControl ) )
            {
                keyPressEvent.setKeyModifier( KeyScanCode::LeftControl );
            }

            if ( KeyboardState::getKeyState( KeyScanCode::LeftShift ) )
            {
                keyPressEvent.setKeyModifier( KeyScanCode::LeftShift );
            }

            pEventDispatcher->dispatchEvent( keyPressEvent );

            break;
        }
        case SDL_EVENT_KEY_UP: {
            KeyboardState::updateState();
            KeyScanCode scanCode = static_cast<KeyScanCode>( e.key.key );
            pEventDispatcher->dispatchEvent( core::KeyReleasedEvent{ scanCode, KeyScanCode::None } );
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            double x = e.motion.x;
            double y = e.motion.y;
            double xRel = e.motion.xrel;
            double yRel = e.motion.yrel;
            pEventDispatcher->dispatchEvent( core::MouseMovedEvent{ x, y, xRel, yRel } );
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            double xOff = e.wheel.x;
            double yOff = e.wheel.y;
            pEventDispatcher->dispatchEvent( core::MouseScrolledEvent{ xOff, yOff } );
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            uint32_t buttonState = SDL_MouseButtonFlags();
            if ( buttonState & SDL_BUTTON_MASK( SDL_BUTTON_LEFT ) )
            {
                core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_MIDDLE ),
                                                      static_cast<int>( MouseAction::Press ),
                                                      static_cast<int>( MouseModifier::None ) };
                pEventDispatcher->dispatchEvent( mouseClicked );
            }
            if ( buttonState & SDL_BUTTON_MASK( SDL_BUTTON_LEFT ) )
            {
                core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_LEFT ),
                                                      static_cast<int>( MouseAction::Press ),
                                                      static_cast<int>( MouseModifier::None ) };
                pEventDispatcher->dispatchEvent( mouseClicked );
            }
            if ( buttonState & SDL_BUTTON_MASK( SDL_BUTTON_RIGHT ) )
            {
                core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_RIGHT ),
                                                      static_cast<int>( MouseAction::Press ),
                                                      static_cast<int>( MouseModifier::None ) };
                pEventDispatcher->dispatchEvent( mouseClicked );
            }
            break;
        }
        default: {
            break;
        }
        }
    }
}

void editor::Editor::run()
{
    core::MainRegistry& mainRegistry = core::MainRegistry::getInstance();
    utilities::TimeTracker* timeTracker = mainRegistry.getTimeTracker();

    timeTracker->start( DELTA_TIME );

    while ( m_running )
    {
        onUpdate();
    }
}

auto editor::Editor::onUpdate() -> void
{
    core::MainRegistry& mainRegistry = core::MainRegistry::getInstance();
    core::AssetManager* assetManager = mainRegistry.getAssetManager();
    physics::JoltPhysics* jolt = mainRegistry.getJoltPhysics();
    utilities::TaskManager* taskManager = mainRegistry.getTaskManager();
    utilities::TimeTracker* timeTracker = mainRegistry.getTimeTracker();
    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();

    float delta = timeTracker->getDurationInSeconds( DELTA_TIME );

    pollEvents();

    jolt->onUpdate( delta );
    taskManager->onUpdate();
    assetManager->onUpdate();
    sceneManager->getCurrentScene()->onUpdate();
    m_vulkanRenderer->onUpdate();

    timeTracker->update( DELTA_TIME );
}

bool editor::Editor::initSDL()
{
    if ( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO ) )
    {
        KERROR( "SDL_Init Error: {}", SDL_GetError() );
        throw std::runtime_error( "SDL_Init failed" );
    }

    SDL_SetHint( SDL_HINT_VULKAN_LIBRARY, "vulkan-1.dll" );

    const char* driver = SDL_GetCurrentVideoDriver();
    KINFO( "Current video driver: {}", driver );

    if ( !SDL_Vulkan_LoadLibrary( nullptr ) )
    {
        KERROR( "SDL Vulkan load failed: {}", SDL_GetError() );
        throw std::runtime_error( "could not load lib vulkan" );
    }

    return true;
}

bool editor::Editor::initMainWindow()
{
    auto& cfg = utilities::EditorConfigManager::getConfig();

    m_window = std::make_unique<window::Window>(
        "kogayonon engine - default project", cfg.width, cfg.height, false, cfg.maximized );
    m_window->setBordered( true );
    m_window->setResizable( true );
    return true;
}

bool editor::Editor::initRenderer()
{
    auto vkCtx = core::MainRegistry::getInstance().getVulkanContext();

    m_vulkanRenderer = std::make_unique<rendering::VulkanRenderer>( vkCtx, m_window->getWindow() );

    if ( !m_vulkanRenderer )
        return false;

    return true;
}

bool editor::Editor::init()
{
    if ( !initSDL() )
    {
        throw std::runtime_error( "sdl could not be initialized" );
    }

    if ( !initMainWindow() )
    {
        throw std::runtime_error( "could not initialize main window" );
    }

    if ( !initMainRegistry() )
    {
        throw std::runtime_error( "main registry could not be initialized" );
    }

    if ( !initRenderer() )
    {
        throw std::runtime_error( "renderer could not be initialized" );
    }

    m_running = true;
    return true;
}

void editor::Editor::onWindowClose( const core::WindowCloseEvent& e )
{
    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    sceneManager->saveScenes();
}

bool editor::Editor::initMainRegistry()
{
    auto& mainRegistry = core::MainRegistry::getInstance();

    std::unique_ptr<graphics::VulkanDevice> device = std::make_unique<graphics::VulkanDevice>( m_window->getWindow() );
    std::unique_ptr<graphics::VulkanSwapchain> swapchain =
        std::make_unique<graphics::VulkanSwapchain>( device.get(), m_window->getWindow() );

#ifdef TRACY_ENABLE
    std::unique_ptr<graphics::VulkanTracyContext> vkTracy =
        std::make_unique<graphics::VulkanTracyContext>( device->getLogicalDevice(),
                                                        device->getPhysicalDevice(),
                                                        device->getGraphicsQueue().handle,
                                                        swapchain->getCommandPool() );
#endif

    std::shared_ptr<graphics::VulkanContext> vkCtx = std::make_shared<graphics::VulkanContext>( graphics::VulkanContext{
        .device = std::move( device ),
        .swapchain = std::move( swapchain ),
#ifdef TRACY_ENABLE
        .tracyContext = std::move( vkTracy ),
#endif
    } );

    KASSERT( vkCtx );
    mainRegistry.addToContext<std::shared_ptr<graphics::VulkanContext>>( std::move( vkCtx ) );

    graphics::VulkanContext* vkCtx_ = mainRegistry.getVulkanContext();
    createDescriptorPool();
    vkCtx_->globalDescriptorPool = m_globalDescriptorPool;

    auto joltPhysics = std::make_shared<physics::JoltPhysics>();
    mainRegistry.addToContext<std::shared_ptr<physics::JoltPhysics>>( std::move( joltPhysics ) );

    auto assetManager = std::make_shared<core::AssetManager>( mainRegistry.getVulkanContext() );
    KASSERT( assetManager && "could not init event dispathcer" );
    mainRegistry.addToContext<std::shared_ptr<core::AssetManager>>( std::move( assetManager ) );

    auto eventDispatcher = std::make_shared<core::EventDispatcher>();
    eventDispatcher->addHandler<core::WindowCloseEvent, &editor::Editor::onWindowClose>( *this );
    KASSERT( eventDispatcher && "could not init event dispathcer" );
    mainRegistry.addToContext<std::shared_ptr<core::EventDispatcher>>( std::move( eventDispatcher ) );

    auto timeTracker = std::make_shared<utilities::TimeTracker>();
    KASSERT( timeTracker && "could not initialize TimeTracker" );
    mainRegistry.addToContext<std::shared_ptr<utilities::TimeTracker>>( std::move( timeTracker ) );

    auto taskManager = std::make_shared<utilities::TaskManager>();
    KASSERT( taskManager && "could not initialize TaskManager" );
    mainRegistry.addToContext<std::shared_ptr<utilities::TaskManager>>( std::move( taskManager ) );

    auto sceneManager = std::make_shared<core::SceneManager>( mainRegistry.getEventDispatcher() );
    // check for the default scene
    std::filesystem::path defaultScenePath =
        std::filesystem::current_path() / "editor" / "scenes" / "defaultScene.kscene";

    sceneManager->addScene( "defaultScene" );
    core::Scene* scene = sceneManager->getCurrentScene();
    core::Entity entity{ scene->getRegistry(), "DefaultCamera" };

    sceneManager->setCurrentScene( scene->getName() );

    // deserialize scene
    if ( std::filesystem::exists( defaultScenePath ) )
    {
        // TODO write this block of code as a json deserializer
        std::ifstream in{ defaultScenePath, std::ios::in | std::ios::binary };
        rapidjson::IStreamWrapper isw{ in };
        rapidjson::Document doc{};
        doc.ParseStream( isw );
        if ( doc.HasParseError() )
        {
            KERROR( "Error at parsing json file for default scene" );
        }

        auto getVec3 = []( const rapidjson::Value& v ) -> glm::vec3 {
            if ( !v.IsArray() || v.Size() != 3 )
                throw std::runtime_error( "Expected array with size 3" );

            return glm::vec3{ v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat() };
        };

        for ( const auto& e : doc["entities"].GetArray() )
        {
            core::Entity ent{ scene->getRegistry() };

            if ( e.HasMember( "identifier" ) )
            {
                std::string name = e["identifier"]["name"].GetString();
                std::string group = e["identifier"]["group"].GetString();

                ent.addComponent<core::IdentifierComponent>(
                    core::IdentifierComponent{ .name = name, .type = core::EntityType::Object, .group = group } );
            }

            if ( e.HasMember( "transform" ) )
            {
                glm::vec3 translation = getVec3( e["transform"]["translation"] );
                glm::vec3 rotation = getVec3( e["transform"]["rotation"] );
                glm::vec3 scale = getVec3( e["transform"]["scale"] );
                core::TransformComponent transform{ .translation = translation, .rotation = rotation, .scale = scale };
                transform.computeMatrix();
                ent.addComponent<core::TransformComponent>( transform );
            }

            if ( e.HasMember( "mesh" ) )
            {
                core::AssetManager* assetManager = mainRegistry.getAssetManager();
                std::filesystem::path meshPath = e["mesh"]["path"].GetString();
                resources::Mesh* mesh = assetManager->loadMesh( meshPath.stem().string(), meshPath.string() );
                ent.addComponent<core::MeshComponent>( core::MeshComponent{ .pMesh = mesh } );
            }
        }
    }

    graphics::VulkanContext* ctx = mainRegistry.getVulkanContext();
    VkExtent2D extent = ctx->swapchain->getSwapchainExtent();

    core::DirectionalLightComponent directionalLight{};
    core::Entity direciontalLightEnt{ scene->getRegistry(), "DefaultDirecitonalLight" };
    direciontalLightEnt.addComponent<core::DirectionalLightComponent>( directionalLight );

    core::PerspectiveCameraComponent cameraComponent{};
    cameraComponent.props.farView = 500.0f;
    cameraComponent.ubo.view =
        glm::lookAt( cameraComponent.props.eye, cameraComponent.props.center, cameraComponent.props.up );
    cameraComponent.ubo.projection = glm::perspective( glm::radians( cameraComponent.props.fov ),
                                                       extent.width / (float)( extent.height ),
                                                       cameraComponent.props.nearView,
                                                       cameraComponent.props.farView );
    cameraComponent.ubo.projection[1][1] *= -1;
    cameraComponent.props.extent = { (float)extent.width, (float)extent.height };
    cameraComponent.isUsed = true;

    entity.addComponent<core::PerspectiveCameraComponent>( cameraComponent );

    KASSERT( sceneManager && "could not initialize SceneManager" );
    mainRegistry.addToContext<std::shared_ptr<core::SceneManager>>( std::move( sceneManager ) );

    return true;
}

void editor::Editor::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes{
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURE_NUM },
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 3000;

    graphics::VulkanContext* vkCtx = core::MainRegistry::getInstance().getVulkanContext();

    VK_CALL( vkCreateDescriptorPool( vkCtx->device->getLogicalDevice(), &poolInfo, nullptr, &m_globalDescriptorPool ) );
}
