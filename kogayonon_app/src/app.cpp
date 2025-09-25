#include "app/app.hpp"
#include <imgui_impl_sdl2.h>
#include <iostream>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/app_event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/input/keyboard_events.hpp"
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

    auto debugWindow = std::make_unique<kogayonon_gui::DebugConsoleWindow>( "Debug console" );
    auto pDbgWin = dynamic_cast<kogayonon_gui::DebugConsoleWindow*>( debugWindow.get() );
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
        kogayonon_core::WindowResizeEvent windowResizeEvent( newWidth, newHeight );
        pEventDispatcher->emitEvent( windowResizeEvent );
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
      pEventDispatcher->emitEvent( keyPressEvent );
      break;
    }
    case SDL_KEYUP: {

      // TODO maybe rethink this a bit
      auto keycode = static_cast<kogayonon_core::KeyCode>( e.key.keysym.sym );
      kogayonon_core::KeyReleasedEvent keyReleaseEvent( keycode );
      pEventDispatcher->emitEvent( keyReleaseEvent );
      break;
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

  glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
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
  auto eventDispatcher = std::make_shared<kogayonon_core::EventDispatcher>();
  assert( eventDispatcher && "could not initialise event manager" );
  mainRegistry.addToContext<std::shared_ptr<kogayonon_core::EventDispatcher>>( std::move( eventDispatcher ) );

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

  auto sceneViewport =
    std::make_unique<kogayonon_gui::SceneViewportWindow>( "Scene", m_pFrameBuffer, playTexture, stopTexture );
  auto fileExplorerWindow = std::make_unique<kogayonon_gui::FileExplorerWindow>( "Assets", folderTexture, fileTexture );
  auto sceneHierarchy = std::make_unique<kogayonon_gui::SceneHierarchyWindow>( "Scene hierarchy" );
  auto performanceWindow = std::make_unique<kogayonon_gui::PerformanceWindow>( "Performance" );
  auto entityPropertiesWindow = std::make_unique<kogayonon_gui::EntityPropertiesWindow>( "Object properties" );

  sceneViewport->setCallback( [this]() { callbackTest(); } );

  IMGUI_MANAGER()->pushWindow( "Scene", std::move( sceneViewport ) );
  IMGUI_MANAGER()->pushWindow( "Object properties", std::move( entityPropertiesWindow ) );
  IMGUI_MANAGER()->pushWindow( "Performance", std::move( performanceWindow ) );
  IMGUI_MANAGER()->pushWindow( "Scene hierarchy", std::move( sceneHierarchy ) );
  IMGUI_MANAGER()->pushWindow( "Assets", std::move( fileExplorerWindow ) );

  return true;
}

bool App::initScenes() const
{
  auto mainScene = std::make_shared<kogayonon_core::Scene>( "Default scene" );

  auto tex = ASSET_MANAGER()->addTexture( "paiangan", "resources/textures/paiangan.png" );
  auto entity = std::make_unique<kogayonon_core::Entity>( mainScene->getRegistry(), "cat texture entity" );
  auto entity2 = std::make_unique<kogayonon_core::Entity>( mainScene->getRegistry(), "slayer texture entity" );
  auto entity3 =
    std::make_unique<kogayonon_core::Entity>( mainScene->getRegistry(), "entity with no component other than name" );
  entity->addComponent<kogayonon_core::TextureComponent>( tex );
  entity2->addComponent<kogayonon_core::TextureComponent>( ASSET_MANAGER()->getTexture( "slayerSword" ) );
  kogayonon_core::SceneManager::addScene( mainScene );

  // set the current scene
  kogayonon_core::SceneManager::setCurrentScene( "Default scene" );
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

  EVENT_DISPATCHER()->addHandler<kogayonon_core::WindowResizeEvent, &App::onWindowResize>( *this );
  return true;
}

void App::rescaleMainViewport( int w, int h )
{
  m_pWindow->resize();
  glViewport( 0, 0, w, h );
}

bool App::onWindowResize( const kogayonon_core::WindowResizeEvent& e )
{
  rescaleMainViewport( e.getWidth(), e.getHeight() );

  // if we need to process the event further (in camera projection matrix for example) we return false
  return true;
}

void App::glDebugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                           const GLchar* message, const void* userParam )
{
  // i care about only medium to high severity for now
  if ( severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM )
  {
    spdlog::error( "Severity-{} Type-{} Source-{} Message-{}", source, type, severity, message );
  }
}

void App::callbackTest() const
{
  // check if the scene ptr is still valid
  if ( auto scene = kogayonon_core::SceneManager::getCurrentScene(); !scene.lock() )
    return;

  // auto& registry = scene.lock()->getRegistry();

  static GLuint quadVAO = 0;
  static GLuint quadVBO = 0;
  if ( quadVAO == 0 )
  {
    float quadVertices[] = {

      // x, y, u, v
      -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
      1.0f,  -1.0f, 1.0f, 0.0f, // bottom-right
      1.0f,  1.0f,  1.0f, 1.0f, // top-right

      -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
      1.0f,  1.0f,  1.0f, 1.0f, // top-right
      -1.0f, 1.0f,  0.0f, 1.0f  // top-left
    };

    glCreateVertexArrays( 1, &quadVAO );
    glCreateBuffers( 1, &quadVBO );

    glNamedBufferStorage( quadVBO, sizeof( quadVertices ), quadVertices, 0 );
    glVertexArrayVertexBuffer( quadVAO, 0, quadVBO, 0, 4 * sizeof( float ) );

    // position attribute
    glEnableVertexArrayAttrib( quadVAO, 0 );
    glVertexArrayAttribFormat( quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0 );
    glVertexArrayAttribBinding( quadVAO, 0, 0 );

    // texcoord attribute
    glEnableVertexArrayAttrib( quadVAO, 1 );
    glVertexArrayAttribFormat( quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ) );
    glVertexArrayAttribBinding( quadVAO, 1, 0 );

    // ensure no element buffer is bound
    GLuint dummy = 0;
    glVertexArrayElementBuffer( quadVAO, dummy );
  }
  SHADER_MANAGER()->bindShader( "3d" );
  glBindVertexArray( quadVAO );
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glBindVertexArray( 0 );
  SHADER_MANAGER()->unbindShader( "3d" );
}
} // namespace kogayonon_app