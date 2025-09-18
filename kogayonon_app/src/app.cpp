#include "app/app.hpp"
#include <glad/glad.h>
#include <imgui_impl_sdl2.h>
#include <memory>
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/app_event.hpp"
#include "core/event/event_manager.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/debug_window.hpp"
#include "gui/file_explorer.hpp"
#include "gui/imgui_manager.hpp"
#include "gui/performance_window.hpp"
#include "gui/scene_hierarchy.hpp"
#include "gui/scene_viewport.hpp"
#include "logger/logger.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/renderer.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/fonts/icons_fontawesome5.h"
#include "utilities/shader_manager/shader_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "window/window.hpp"

using namespace kogayonon_logger;

namespace kogayonon_app
{
App::~App()
{
  cleanup();
}

void App::cleanup()
{
  Logger::info( "Closing app and cleaning up" );
  Logger::shutdown();
}

void App::pollEvents()
{
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
        kogayonon_core::WindowResizeEvent windowResizeEvent( newWidth, newHeight );
        EVENT_MANAGER()->dispatchEventToListeners( windowResizeEvent );
      }
      break;
    }
    case SDL_QUIT: {
      m_running = false;
      break;
    }
    case SDL_KEYDOWN: {

      // TODO maybe rethink this a bit
      auto keycode = static_cast<kogayonon_core::KeyCode>( e.key.keysym.sym );
      kogayonon_core::KeyPressedEvent keyPressEvent( keycode, 0 );
      EVENT_MANAGER()->dispatchEventToListeners( keyPressEvent );
      break;
    }
    case SDL_KEYUP: {

      // TODO maybe rethink this a bit
      auto keycode = static_cast<kogayonon_core::KeyCode>( e.key.keysym.sym );
      kogayonon_core::KeyReleasedEvent keyReleaseEvent( keycode );
      EVENT_MANAGER()->dispatchEventToListeners( keyReleaseEvent );
      break;
    }
    }
  }
}

void App::run()
{
  if ( !init() )
  {
    m_running = false;
  }

  TIME_TRACKER()->start( "deltaTime" );

  while ( m_running )
  {
    TIME_TRACKER()->update( "deltaTime" );
    pollEvents();
    IMGUI_MANAGER()->draw();
    m_pWindow->swapWindow();
  }
}

bool App::initSDL()
{
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    Logger::critical( "Could not initialize sdl" );
    return false;
  }

  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 6 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
  auto pWinProps = m_pWindow->getWindowProps();

  auto flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  if ( pWinProps->maximized )
  {
    flags |= SDL_WINDOW_MAXIMIZED;
  }

  auto win = SDL_CreateWindow( pWinProps->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pWinProps->width,
                               pWinProps->height, flags );
  m_pWindow->setWindow( std::move( win ) );

  auto ctx = SDL_GL_CreateContext( m_pWindow->getWindow() );
  m_pWindow->setContext( std::move( ctx ) );

  SDL_GL_MakeCurrent( m_pWindow->getWindow(), m_pWindow->getContext() );

  if ( !gladLoadGLLoader( (GLADloadproc)SDL_GL_GetProcAddress ) )
  {
    Logger::critical( "Failed to initialize GLAD" );
    return false;
  }

#ifdef _DEBUG
  glEnable( GL_DEBUG_OUTPUT );
#endif
  glCullFace( GL_CCW );
  glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
  rescaleMainViewport( pWinProps->width, pWinProps->height );
  return true;
}

bool App::initRegistries()
{
  auto& mainRegistry = REGISTRY();

  // init renderer
  auto renderer = std::make_shared<kogayonon_rendering::Renderer>();
  renderer->initialise();
  assert( renderer && "could not initialise renderer" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_rendering::Renderer>>( std::move( renderer ) );

  // init time tracker
  auto timeTracker = std::make_shared<kogayonon_utilities::TimeTracker>();
  assert( timeTracker && "could not initialise time tracker" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::TimeTracker>>( std::move( timeTracker ) );

  // init imgui manager
  auto imguiManager = std::make_shared<kogayonon_gui::ImGuiManager>( m_pWindow->getWindow(), m_pWindow->getContext() );
  assert( imguiManager && "could not initialise imgui manager" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>( std::move( imguiManager ) );

  // init event manager
  auto eventManager = std::make_shared<kogayonon_core::EventManager>();
  assert( eventManager && "could not initialise event manager" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_core::EventManager>>( std::move( eventManager ) );

  // init task manager
  auto taskManager = std::make_shared<kogayonon_utilities::TaskManager>( 10 );
  assert( taskManager && "could not initialise task manager" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::TaskManager>>( std::move( taskManager ) );

  // init shader manager
  auto shaderManager = std::make_shared<kogayonon_utilities::ShaderManager>();
  assert( shaderManager && "could not initialise shader manager" );
  shaderManager->pushShader( "resources/shaders/3d_vertex.glsl", "resources/shaders/3d_fragment.glsl", "3d" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::ShaderManager>>( std::move( shaderManager ) );

  // init asset manager
  auto assetManager = std::make_shared<kogayonon_utilities::AssetManager>();
  assert( assetManager && "could not initialise asset manager" );

  assetManager->addTexture( "play", "resources/textures/play.png" );
  assetManager->addTexture( "stop", "resources/textures/stop.png" );

  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::AssetManager>>( std::move( assetManager ) );

  return true;
}

bool App::initGui()
{
  // insert windows
  m_pFrameBuffer = std::make_shared<kogayonon_rendering::FrameBuffer>( 400, 400 );
  auto playTexture = ASSET_MANAGER()->getTexture( "play" ).lock()->getTextureId();
  auto stopTexture = ASSET_MANAGER()->getTexture( "stop" ).lock()->getTextureId();
  auto sceneViewport = std::make_unique<kogayonon_gui::SceneViewportWindow>( ICON_FA_IMAGE " Scene", m_pFrameBuffer,
                                                                             playTexture, stopTexture );
  sceneViewport->setCallback( [this]() { callbackTest(); } );

  auto debugWindow = std::make_unique<kogayonon_gui::DebugConsoleWindow>( "Debug console" );

  // root for where the file explorer can see files or folders
  std::string rootPath = "/";
  auto fileExplorerWindow = std::make_unique<kogayonon_gui::FileExplorerWindow>( "Assets", std::move( rootPath ) );

  auto sceneHierarchy = std::make_unique<kogayonon_gui::SceneHierarchyWindow>( "Scene hierarchy" );

  auto performanceWindow = std::make_unique<kogayonon_gui::PerformanceWindow>( "Performance" );

  IMGUI_MANAGER()->pushWindow( "Scene", std::move( sceneViewport ) );
  IMGUI_MANAGER()->pushWindow( "Performance", std::move( performanceWindow ) );
  IMGUI_MANAGER()->pushWindow( "Scene hierarchy", std::move( sceneHierarchy ) );
  IMGUI_MANAGER()->pushWindow( "Debug console", std::move( debugWindow ) );
  IMGUI_MANAGER()->pushWindow( "Assets", std::move( fileExplorerWindow ) );

  return true;
}

bool App::initScenes()
{
  auto mainScene = std::make_shared<kogayonon_core::Scene>( "Default scene" );

  // add a test entity with a texture component
  auto entity = std::make_unique<kogayonon_core::Entity>( mainScene->getRegistry(), "test texture from model" );

  auto tex = ASSET_MANAGER()->addTexture( "textureTest", "resources/textures/paiangan.png" );
  entity->addComponent<kogayonon_core::TextureComponent>( tex );
  kogayonon_core::SceneManager::getInstance().addScene( mainScene );

  // set the current scene
  kogayonon_core::SceneManager::getInstance().setCurrentScene( "Default scene" );
  return true;
}

bool App::init()
{
#ifdef _DEBUG
  m_pWindow = std::make_shared<kogayonon_window::Window>( "kogayonon engine (DEBUG)", 1200, 800, 1, false );
#else
  m_pWindow = std::make_shared<kogayonon_window::Window>( "kogayonon engine", 800, 600, 1, false );
#endif

  Logger::initialize( "log.txt" );

  if ( !initSDL() )
  {
    return false;
  }

  if ( !initRegistries() )
  {
    return false;
  }

  if ( !initGui() )
  {
    return false;
  }

  if ( !initScenes() )
  {
    return false;
  }

  EVENT_MANAGER()->listenToEvent<kogayonon_core::WindowResizeEvent>( [this]( const kogayonon_core::Event& e ) -> bool {
    return this->onWindowResize( (kogayonon_core::WindowResizeEvent&)e );
  } );

  return true;
}

void App::rescaleMainViewport( int w, int h )
{
  m_pWindow->resize();
  glViewport( 0, 0, w, h );
}

bool App::onWindowResize( kogayonon_core::WindowResizeEvent& e )
{
  rescaleMainViewport( e.getWidth(), e.getHeight() );

  // if we need to process the event further (in camera projection matrix for example) we return false
  return true;
}

void App::callbackTest()
{
  auto& sceneManager = kogayonon_core::SceneManager::getInstance();
  auto scene = sceneManager.getCurrentScene();

  // check if the scene ptr is still valid
  if ( !scene.lock() )
    return;

  auto& registry = scene.lock()->getRegistry();

  SHADER_MANAGER()->bindShader( "3d" );
  glBindVertexArray( 0 );
  SHADER_MANAGER()->unbindShader( "3d" );
}
} // namespace kogayonon_app