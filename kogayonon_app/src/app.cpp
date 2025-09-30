#include "app/app.hpp"
#include <imgui_impl_sdl2.h>
#include <iostream>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/app_event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/input/mouse_codes.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/debug_window.hpp"
#include "gui/entity_properties.hpp"
#include "gui/file_explorer.hpp"
#include "gui/imgui_manager.hpp"
#include "gui/performance_window.hpp"
#include "gui/scene_hierarchy.hpp"
#include "gui/scene_viewport.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/renderer.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "window/window.hpp"

using namespace kogayonon_core;

namespace kogayonon_app
{
App::App()
{
  try
  {
    // had to do all this to setup the debug console print
    auto defferedSink = std::make_shared<kogayonon_gui::DeferredImGuiSink<std::mutex>>();
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>( "logs/basic-log.txt", true );
    std::vector<spdlog::sink_ptr> sinks{ fileSink, defferedSink };

    auto logger = std::make_shared<spdlog::logger>( "app_logger", sinks.begin(), sinks.end() );
    spdlog::set_level( spdlog::level::debug );
    spdlog::set_pattern( "[%H:%M:%S] [%^%L%$] %v" );
    spdlog::set_default_logger( logger );

    if ( !init() )
    {
      m_running = false;
    }

    auto debugWindow = std::make_unique<kogayonon_gui::DebugConsoleWindow>( "Debug console##win" );
    auto pDbgWin = debugWindow.get();
    defferedSink->setWindow( pDbgWin );
    IMGUI_MANAGER()->pushWindow( "Debug console", std::move( debugWindow ) );
  }
  catch ( const spdlog::spdlog_ex& ex )
  {
    std::cout << "Logger init failed " << ex.what();
  }
}

App::~App()
{
  cleanup();
}

void App::cleanup() const
{
  spdlog::info( "Closing app and cleaning up" );
}

void App::pollEvents()
{
  auto& pEventDispatcher = EVENT_DISPATCHER();
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
        WindowResizeEvent windowResizeEvent{ newWidth, newHeight };
        pEventDispatcher->emitEvent( windowResizeEvent );
      }
      break;
    }
    case SDL_QUIT: {
      m_running = false;
      break;
    }
    case SDL_KEYDOWN: {
      auto keycode = static_cast<KeyCode>( e.key.keysym.sym );
      KeyPressedEvent keyPressEvent{ keycode, 0 };
      pEventDispatcher->emitEvent( keyPressEvent );
      break;
    }
    case SDL_KEYUP: {
      auto keycode = static_cast<KeyCode>( e.key.keysym.sym );
      KeyReleasedEvent keyReleaseEvent{ keycode };
      pEventDispatcher->emitEvent( keyReleaseEvent );
      break;
    }
    case SDL_MOUSEMOTION: {
      double x = e.motion.x;
      double y = e.motion.y;
      double xRel = e.motion.xrel;
      double yRel = e.motion.yrel;
      MouseMovedEvent mouseMovedEvent{ x, y, xRel, yRel };
      pEventDispatcher->emitEvent( mouseMovedEvent );
      break;
    }
    case SDL_MOUSEWHEEL: {
      double xOff = e.wheel.x;
      double yOff = e.wheel.y;
      MouseScrolledEvent mouseScrolled{ xOff, yOff };
      pEventDispatcher->emitEvent( mouseScrolled );
    }
    case SDL_MOUSEBUTTONDOWN: {
      UINT32 buttonState = SDL_GetMouseState( NULL, NULL );
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_MIDDLE ) )
      {
        MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_MIDDLE ),
                                        static_cast<int>( MouseAction::Press ),
                                        static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->emitEvent( mouseClicked );
      }
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_LEFT ) )
      {
        MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_LEFT ),
                                        static_cast<int>( MouseAction::Press ),
                                        static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->emitEvent( mouseClicked );
      }
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_RIGHT ) )
      {
        MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_RIGHT ),
                                        static_cast<int>( MouseAction::Press ),
                                        static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->emitEvent( mouseClicked );
      }
    }
    }
  }
}

void App::run()
{
  const auto& pTimeTracker = TIME_TRACKER();
  const auto& pImGuiManager = IMGUI_MANAGER();

  pTimeTracker->start( "deltaTime" );

  while ( m_running )
  {
    pTimeTracker->update( "deltaTime" );
    pollEvents();
    pImGuiManager->draw();
    m_pWindow->swapWindow();
  }
}

bool App::initSDL()
{
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    spdlog::critical( "Could not initialize sdl" );
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
    spdlog::critical( "Failed to initialize GLAD" );
    return false;
  }

#ifdef _DEBUG
  glEnable( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( glDebugCallback, nullptr );
#endif
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  rescaleMainViewport( pWinProps->width, pWinProps->height );
  return true;
}

bool App::initRegistries() const
{
  auto& mainRegistry = REGISTRY();

  // init time tracker
  auto timeTracker = std::make_shared<kogayonon_utilities::TimeTracker>();
  assert( timeTracker && "could not initialise time tracker" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::TimeTracker>>( std::move( timeTracker ) );

  // init imgui manager
  auto imguiManager = std::make_shared<kogayonon_gui::ImGuiManager>( m_pWindow->getWindow(), m_pWindow->getContext() );
  assert( imguiManager && "could not initialise imgui manager" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_gui::ImGuiManager>>( std::move( imguiManager ) );

  // init event manager
  auto eventDispatcher = std::make_shared<EventDispatcher>();
  assert( eventDispatcher && "could not initialise event manager" );
  mainRegistry.addToContext<std::shared_ptr<EventDispatcher>>( std::move( eventDispatcher ) );

  // init task manager
  auto taskManager = std::make_shared<kogayonon_utilities::TaskManager>( 10 );
  assert( taskManager && "could not initialise task manager" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::TaskManager>>( std::move( taskManager ) );

  // init shader manager
  auto shaderManager = std::make_shared<kogayonon_utilities::ShaderManager>();
  assert( shaderManager && "could not initialise shader manager" );
  shaderManager->pushShader( "resources/shaders/3d_vertex.glsl", "resources/shaders/3d_fragment.glsl", "3d" );
  shaderManager->pushShader( "resources/shaders/white_vertex.glsl", "resources/shaders/white_fragment.glsl", "white" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::ShaderManager>>( std::move( shaderManager ) );

  // init asset manager
  auto assetManager = std::make_shared<kogayonon_utilities::AssetManager>();
  assert( assetManager && "could not initialise asset manager" );

  assetManager->addTexture( "slayerSword", "resources/textures/slayer_sword.png" );
  assetManager->addTexture( "play", "resources/textures/play.png" );
  assetManager->addTexture( "stop", "resources/textures/stop.png" );
  assetManager->addTexture( "file", "resources/textures/file.png" );
  assetManager->addTexture( "folder", "resources/textures/folder.png" );
  assetManager->addTexture( "default", "resources/textures/default.png" );

  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::AssetManager>>( std::move( assetManager ) );

  return true;
}

bool App::initGui()
{
  // frame buffer for the scene viewport
  m_pFrameBuffer = std::make_shared<kogayonon_rendering::FrameBuffer>( 400, 400 );

  const auto& pAssetManager = ASSET_MANAGER();

  // textures for imgui buttons or windows and what not
  auto playTexture = pAssetManager->getTexture( "play" ).lock()->getTextureId();
  auto stopTexture = pAssetManager->getTexture( "stop" ).lock()->getTextureId();
  auto fileTexture = pAssetManager->getTexture( "file" ).lock()->getTextureId();
  auto folderTexture = pAssetManager->getTexture( "folder" ).lock()->getTextureId();

  auto sceneViewport = std::make_unique<kogayonon_gui::SceneViewportWindow>( m_pWindow->getWindow(), "Scene##win",
                                                                             m_pFrameBuffer, playTexture, stopTexture );
  auto fileExplorerWindow =
    std::make_unique<kogayonon_gui::FileExplorerWindow>( "Assets##win", folderTexture, fileTexture );
  auto sceneHierarchy = std::make_unique<kogayonon_gui::SceneHierarchyWindow>( "Scene hierarchy##win" );
  auto performanceWindow = std::make_unique<kogayonon_gui::PerformanceWindow>( "Performance##win" );
  auto entityPropertiesWindow = std::make_unique<kogayonon_gui::EntityPropertiesWindow>( "Object properties##win" );

  IMGUI_MANAGER()->pushWindow( "Scene", std::move( sceneViewport ) );
  IMGUI_MANAGER()->pushWindow( "Object properties", std::move( entityPropertiesWindow ) );
  IMGUI_MANAGER()->pushWindow( "Performance", std::move( performanceWindow ) );
  IMGUI_MANAGER()->pushWindow( "Scene hierarchy", std::move( sceneHierarchy ) );
  IMGUI_MANAGER()->pushWindow( "Assets", std::move( fileExplorerWindow ) );

  return true;
}

bool App::initScenes() const
{
  auto mainScene = std::make_shared<Scene>( "Default scene" );
  SceneManager::addScene( mainScene );

  // set the current scene
  SceneManager::setCurrentScene( "Default scene" );
  return true;
}

bool App::init()
{
#ifdef _DEBUG
  m_pWindow = std::make_shared<kogayonon_window::Window>( "kogayonon engine (DEBUG)", 1800, 1000, 1, true );
#else
  m_pWindow = std::make_shared<kogayonon_window::Window>( "kogayonon engine", 1800, 1000, 1, false );
#endif

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

  EVENT_DISPATCHER()->addHandler<WindowResizeEvent, &App::onWindowResize>( *this );
  return true;
}

void App::rescaleMainViewport( int w, int h )
{
  m_pWindow->resize();
  glViewport( 0, 0, w, h );
}

bool App::onWindowResize( const WindowResizeEvent& e )
{
  rescaleMainViewport( e.getWidth(), e.getHeight() );

  // if we need to process the event further (in camera projection matrix for example) we return false
  return true;
}

static const char* glSourceToStr( GLenum source )
{
  switch ( source )
  {
  case GL_DEBUG_SOURCE_API:
    return "API";
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    return "Window System";
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    return "Shader Compiler";
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    return "Third Party";
  case GL_DEBUG_SOURCE_APPLICATION:
    return "Application";
  case GL_DEBUG_SOURCE_OTHER:
    return "Other";
  default:
    return "Unknown";
  }
}

static const char* glTypeToStr( GLenum type )
{
  switch ( type )
  {
  case GL_DEBUG_TYPE_ERROR:
    return "Error";
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    return "Deprecated Behaviour";
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    return "Undefined Behaviour";
  case GL_DEBUG_TYPE_PORTABILITY:
    return "Portability";
  case GL_DEBUG_TYPE_PERFORMANCE:
    return "Performance";
  case GL_DEBUG_TYPE_MARKER:
    return "Marker";
  case GL_DEBUG_TYPE_PUSH_GROUP:
    return "Push Group";
  case GL_DEBUG_TYPE_POP_GROUP:
    return "Pop Group";
  case GL_DEBUG_TYPE_OTHER:
    return "Other";
  default:
    return "Unknown";
  }
}

static const char* glSeverityToStr( GLenum severity )
{
  switch ( severity )
  {
  case GL_DEBUG_SEVERITY_HIGH:
    return "High";
  case GL_DEBUG_SEVERITY_MEDIUM:
    return "Medium";
  case GL_DEBUG_SEVERITY_LOW:
    return "Low";
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    return "Notification";
  default:
    return "Unknown";
  }
}

void App::glDebugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                           const GLchar* message, const void* userParam )
{
  // i care about only medium to high severity for now
  if ( severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM )
  {
    spdlog::error( "[OpenGL Debug] Severity: {} | Type: {} | Source: {} | ID: {} | Message: {}",
                   glSeverityToStr( severity ), glTypeToStr( type ), glSourceToStr( source ), id, message );
  }
}
} // namespace kogayonon_app