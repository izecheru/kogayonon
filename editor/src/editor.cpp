
#include "editor/editor.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/camera_component.hpp"
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
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <rapidjson/istreamwrapper.h>

editor::Editor::Editor()
{
  utilities::EditorConfigManager::initConfig();

  KeyboardState::initState();

  init();
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
    ImGui_ImplSDL2_ProcessEvent( &e );
    switch ( e.type )
    {
    case SDL_WINDOWEVENT: {
      if ( e.window.event == SDL_WINDOWEVENT_RESIZED )
      {
        int newWidth = e.window.data1;
        int newHeight = e.window.data2;
        K_INFO( "Resized wind" );
        pEventDispatcher->dispatchEvent( core::WindowResizeEvent{ newWidth, newHeight } );
      }

      if ( e.window.event == SDL_WINDOWEVENT_RESTORED )
        K_INFO( "Restored wind" );

      break;
    }
    case SDL_QUIT: {
      pEventDispatcher->dispatchEvent( core::WindowCloseEvent{} );
      m_running = false;
      break;
    }
    case SDL_KEYDOWN: {
      KeyboardState::updateState();
      auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );

      auto keyPressEvent = core::KeyPressedEvent{ scanCode, KeyScanCode::None, 0 };

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
    case SDL_KEYUP: {
      KeyboardState::updateState();
      auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );
      pEventDispatcher->dispatchEvent( core::KeyReleasedEvent{ scanCode, KeyScanCode::None } );
      break;
    }
    case SDL_MOUSEMOTION: {
      double x = e.motion.x;
      double y = e.motion.y;
      double xRel = e.motion.xrel;
      double yRel = e.motion.yrel;
      pEventDispatcher->dispatchEvent( core::MouseMovedEvent{ x, y, xRel, yRel } );
      break;
    }
    case SDL_MOUSEWHEEL: {
      double xOff = e.wheel.x;
      double yOff = e.wheel.y;
      pEventDispatcher->dispatchEvent( core::MouseScrolledEvent{ xOff, yOff } );
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      uint32_t buttonState = SDL_GetMouseState( NULL, NULL );
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_MIDDLE ) )
      {
        core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_MIDDLE ),
                                              static_cast<int>( MouseAction::Press ),
                                              static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->dispatchEvent( mouseClicked );
      }
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_LEFT ) )
      {
        core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_LEFT ),
                                              static_cast<int>( MouseAction::Press ),
                                              static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->dispatchEvent( mouseClicked );
      }
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_RIGHT ) )
      {
        core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_RIGHT ),
                                              static_cast<int>( MouseAction::Press ),
                                              static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->dispatchEvent( mouseClicked );
      }
      break;
    }
    default:
      break;
    }
  }
}

void editor::Editor::run()
{
  auto vkContext = core::MainRegistry::getInstance().getVulkanContext();
  auto& swapchain = vkContext->swapchain;

  auto assetManager = core::MainRegistry::getInstance().getAssetManager();
  assetManager->initSampler();

  auto& mainRegistry = core::MainRegistry::getInstance();
  auto timeTracker = mainRegistry.getTimeTracker();
  // Start the delta time count
  timeTracker->start( DELTA_TIME );

  while ( m_running )
  {
    onUpdate();
  }
}

auto editor::Editor::onUpdate() -> void
{
  graphics::VulkanContext* vkCtx = core::MainRegistry::getInstance().getVulkanContext();
  core::MainRegistry& mainRegistry = core::MainRegistry::getInstance();
  physics::JoltPhysics* jolt = mainRegistry.getJoltPhysics();
  utilities::TaskManager* taskManager = mainRegistry.getTaskManager();
  utilities::TimeTracker* timeTracker = mainRegistry.getTimeTracker();

  taskManager->onUpdate();
  // Poll events and dispatch them to listeners
  pollEvents();

  // Update physics based on delta
  jolt->onUpdate( timeTracker->getDurationInSeconds( DELTA_TIME ) );

  // Render
  m_pRenderer->render();
  vkCtx->swapchain->presentFrame();

  // Update delta time
  timeTracker->update( DELTA_TIME );
}

bool editor::Editor::initSDL()
{
  if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ) != 0 )
  {
    K_ERROR( "SDL_Init Error: {}", SDL_GetError() );
    throw std::runtime_error( "SDL_Init failed" );
  }

  if ( SDL_Vulkan_LoadLibrary( nullptr ) != 0 )
  {
    K_ERROR( "SDL Vulkan load failed: {}", SDL_GetError() );
    throw std::runtime_error( "could not load lib vulkan" );
  }

  return true;
}

bool editor::Editor::initVulkan()
{
  auto vkCtx = core::MainRegistry::getInstance().getVulkanContext();

  createDescriptorPool();
  vkCtx->globalDescriptorPool = m_globalDescriptorPool;

  // TODO(kogayonon) this does not look clean, make the asset manager ctor better or smth
  auto assetManager = core::MainRegistry::getInstance().getAssetManager();
  assetManager->setContext( vkCtx );
  assetManager->setDescriptorPool( m_globalDescriptorPool );
  assetManager->initDescriptors();

  return true;
}

bool editor::Editor::initMainWindow()
{
  auto& cfg = utilities::EditorConfigManager::getConfig();

  m_pWindow = std::make_shared<window::Window>( "kogayonon engine", cfg.width, cfg.height, false, cfg.maximized );
  m_pWindow->setBordered( true );
  m_pWindow->setResizable( true );
  return true;
}

bool editor::Editor::initRenderer()
{
  auto vkCtx = core::MainRegistry::getInstance().getVulkanContext();

  m_pRenderer = std::make_shared<rendering::VulkanRenderer>( vkCtx, m_pWindow->getWindow() );

  if ( !m_pRenderer )
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

  if ( !initVulkan() )
  {
    throw std::runtime_error( "vulkan could not be initialized" );
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
}

bool editor::Editor::initMainRegistry()
{
  auto& mainRegistry = core::MainRegistry::getInstance();

  auto device = std::make_unique<graphics::VulkanDevice>( m_pWindow->getWindow() );
  auto swapchain = std::make_unique<graphics::VulkanSwapchain>( device.get(), m_pWindow->getWindow() );

#ifdef TRACY_ENABLE
  auto vkTracy = std::make_unique<graphics::VulkanTracyContext>();
  vkTracy->initCtx( device->getLogicalDevice(),
                    device->getPhysicalDevice(),
                    device->getGraphicsQueue().handle,
                    swapchain->getCommandPool() );
#endif

  std::unique_ptr<graphics::VulkanContext> vkCtx =
    std::make_unique<graphics::VulkanContext>( graphics::VulkanContext{ .device = std::move( device ),
                                                                        .swapchain = std::move( swapchain ),
#ifdef TRACY_ENABLE
                                                                        .tracyContext = std::move( vkTracy )
#endif

    } );

  mainRegistry.addToContext<std::shared_ptr<graphics::VulkanContext>>( std::move( vkCtx ) );

  auto joltPhysics = std::make_shared<physics::JoltPhysics>();
  mainRegistry.addToContext<std::shared_ptr<physics::JoltPhysics>>( std::move( joltPhysics ) );

  auto assetManager = std::make_shared<core::AssetManager>();
  K_ASSERT( assetManager && "could not init event dispathcer" );
  mainRegistry.addToContext<std::shared_ptr<core::AssetManager>>( std::move( assetManager ) );

  auto eventDispatcher = std::make_shared<core::EventDispatcher>();
  eventDispatcher->addHandler<core::WindowCloseEvent, &editor::Editor::onWindowClose>( *this );
  K_ASSERT( eventDispatcher && "could not init event dispathcer" );
  mainRegistry.addToContext<std::shared_ptr<core::EventDispatcher>>( std::move( eventDispatcher ) );

  auto timeTracker = std::make_shared<utilities::TimeTracker>();
  K_ASSERT( timeTracker && "could not initialize TimeTracker" );
  mainRegistry.addToContext<std::shared_ptr<utilities::TimeTracker>>( std::move( timeTracker ) );

  auto taskManager = std::make_shared<utilities::TaskManager>();
  K_ASSERT( taskManager && "could not initialize TaskManager" );
  mainRegistry.addToContext<std::shared_ptr<utilities::TaskManager>>( std::move( taskManager ) );

  auto sceneManager = std::make_shared<core::SceneManager>( mainRegistry.getEventDispatcher() );
  sceneManager->addScene( "defaultScene" );
  auto scene = sceneManager->getCurrentScene();
  core::Entity entity{ scene->getRegistry(), "DefaultCamera" };
  sceneManager->setCurrentScene( scene->getName() );

  K_ASSERT( sceneManager && "could not initialize SceneManager" );
  mainRegistry.addToContext<std::shared_ptr<core::SceneManager>>( std::move( sceneManager ) );

  auto ctx = mainRegistry.getVulkanContext();
  auto extent = ctx->swapchain->getSwapchainExtent();

  auto cameraComponent = core::PerspectiveCameraComponent{};

  cameraComponent.props.farView = 3000.0f;

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

  return true;
}

void editor::Editor::createDescriptorPool()
{
  std::vector<VkDescriptorPoolSize> poolSizes{
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 500 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 500 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURE_NUM },
  };

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
  poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 3000;

  auto vkCtx = core::MainRegistry::getInstance().getVulkanContext();

  VK_CALL( vkCreateDescriptorPool( vkCtx->device->getLogicalDevice(), &poolInfo, nullptr, &m_globalDescriptorPool ) );
}
