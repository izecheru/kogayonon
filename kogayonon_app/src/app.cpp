#include "app/app.hpp"
#include <core/ecs/components/directional_light_component.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_impl_sdl2.h>
#include <iostream>
#include <memory>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/pointlight_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/app_event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/project_event.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/project/project_manager.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/debug_window.hpp"
#include "gui/entity_properties.hpp"
#include "gui/file_explorer.hpp"
#include "gui/imgui_manager.hpp"
#include "gui/performance_window.hpp"
#include "gui/project_window.hpp"
#include "gui/scene_hierarchy.hpp"
#include "gui/scene_viewport.hpp"
#include "physics/nvidia_physx.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/configurator/configurator.hpp"
#include "utilities/input/mouse_codes.hpp"
#include "utilities/jsoner/jsoner.hpp"
#include "utilities/serializer/serializer.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "window/window.hpp"

using namespace kogayonon_core;
using namespace kogayonon_physics;

namespace kogayonon_app
{
App::App()
{
  try
  {
    // Console sink (all levels)
    auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
    consoleSink->set_level( spdlog::level::debug ); // log everything to console

    // File sink (only error and above)
    auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_st>( "logs/log.txt", 23, 59 );
    fileSink->set_level( spdlog::level::err ); // only error+

    // Collect sinks
    std::vector<spdlog::sink_ptr> sinks{ consoleSink, fileSink };

    // Create logger with both sinks
    auto logger = std::make_shared<spdlog::logger>( "app_logger", sinks.begin(), sinks.end() );

    // Set global settings
    logger->set_level( spdlog::level::debug ); // logger accepts everything, sinks decide what to filter
    logger->set_pattern( "[%H:%M:%S] [%^%L%$] %v" );

    spdlog::set_default_logger( logger );

    // parse the config and fill the json document
    Configurator::parseConfigFile();

    if ( !init() )
    {
      m_running = false;
    }
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
  NvidiaPhysx::getInstance().releasePhysx();
}

void App::pollEvents()
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
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
      pEventDispatcher->emitEvent( WindowCloseEvent{} );
      break;
    }
    case SDL_KEYDOWN: {
      KeyboardState::updateState();
      auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );

      KeyPressedEvent keyPressEvent{ scanCode, KeyScanCode::None, 0 };
      if ( KeyboardState::getKeyState( KeyScanCode::LeftControl ) )
      {
        keyPressEvent.setKeyModifier( KeyScanCode::LeftControl );
      }

      if ( KeyboardState::getKeyState( KeyScanCode::LeftShift ) )
      {
        keyPressEvent.setKeyModifier( KeyScanCode::LeftShift );
      }

      pEventDispatcher->emitEvent( keyPressEvent );
      break;
    }
    case SDL_KEYUP: {
      KeyboardState::updateState();
      auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );
      KeyReleasedEvent keyReleaseEvent{ scanCode, KeyScanCode::None };
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
      break;
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
      break;
    }
    default:
      break;
    }
  }
}

void App::run()
{
  const auto& pTimeTracker = MainRegistry::getInstance().getTimeTracker();
  const auto& pImGuiManager = MainRegistry::getInstance().getImGuiManager();
  auto& nvidiaPhysics = NvidiaPhysx::getInstance();

  pTimeTracker->start( "deltaTime" );

  while ( m_running )
  {
    pTimeTracker->update( "deltaTime" );
    pollEvents();
    if ( nvidiaPhysics.isRunning() )
    {
      nvidiaPhysics.simulate( pTimeTracker->getDuration( "deltaTime" ).count() );
    }
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
  SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
  auto pWinProps = m_pWindow->getWindowProps();

  auto flags = SDL_WINDOW_OPENGL;

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

  m_pWindow->setBordered( false );
  m_pWindow->setResizable( true );

#ifdef _DEBUG
  glEnable( GL_DEBUG_OUTPUT );
  glDebugMessageControl( GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE );
  glDebugMessageCallback( glDebugCallback, nullptr );
#endif
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_STENCIL_TEST );
  glDepthFunc( GL_LESS );
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  glFrontFace( GL_CCW );
  rescaleMainViewport( pWinProps->width, pWinProps->height );
  return true;
}

bool App::initRegistries() const
{
  auto& mainRegistry = MainRegistry::getInstance();

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
  shaderManager->pushShader( "resources/shaders/3d_normal_vert.glsl", "resources/shaders/3d_normal_frag.glsl",
                             "3d_normal" );
  shaderManager->pushShader( "resources/shaders/picking_vertex.glsl", "resources/shaders/picking_fragment.glsl",
                             "picking" );
  shaderManager->pushShader( "resources/shaders/depth_vert.glsl", "resources/shaders/depth_frag.glsl", "depth" );
  shaderManager->pushShader( "resources/shaders/depth_vert.glsl", "resources/shaders/depth_debug_frag.glsl",
                             "depthDebug" );
  shaderManager->pushShader( "resources/shaders/outlining_vert.glsl", "resources/shaders/outlining_frag.glsl",
                             "outlining" );

  shaderManager->compileMarkedShaders();

  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::ShaderManager>>( std::move( shaderManager ) );

  // init asset manager and add all default icons and textures and whatnot
  auto& assetManager = AssetManager::getInstance();

  assetManager.addTexture( "play.png" );
  assetManager.addTexture( "stop.png" );
  assetManager.addTexture( "file.png" );
  assetManager.addTexture( "folder.png" );
  assetManager.addTexture( "default.png" );
  assetManager.addTexture( "3d-cube.png" );
  assetManager.addTexture( "logo.png" );
  assetManager.addTexture( "render_mode_icon.png" );
  assetManager.addTexture( "gltf_icon.png" );
  assetManager.addTexture( "txt_icon.png" );
  assetManager.addTexture( "shader_icon.png" );
  assetManager.addTexture( "png_icon.png" );

  // ugly init but it is what it is
  auto& physics = NvidiaPhysx::getInstance();

  return true;
}

bool App::initGui()
{
  auto imgui = MainRegistry::getInstance().getImGuiManager();
  auto projectWindow = std::make_unique<kogayonon_gui::ProjectWindow>( "Project" );
  imgui->pushWindow( "Project", std::move( projectWindow ) );

  return true;
}

bool App::initGuiForProject()
{
  const auto& config = Configurator::getConfig();

  // enable border
  m_pWindow->setBordered( true );

  // can resize the window now
  m_pWindow->setResizable( true );

  // maximize window
  if ( config.maximized )
  {
    m_pWindow->maximize();
  }

  auto& assetManager = AssetManager::getInstance();

  // textures for imgui buttons or windows and what not
  auto playTexture = assetManager.getTexture( "play.png" ).lock()->getTextureId();
  auto stopTexture = assetManager.getTexture( "stop.png" ).lock()->getTextureId();
  auto fileTexture = assetManager.getTexture( "file.png" ).lock()->getTextureId();
  auto folderTexture = assetManager.getTexture( "folder.png" ).lock()->getTextureId();

  auto sceneViewport = std::make_unique<kogayonon_gui::SceneViewportWindow>( m_pWindow->getWindow(), "Viewport",
                                                                             playTexture, stopTexture );
  auto fileExplorerWindow = std::make_unique<kogayonon_gui::FileExplorerWindow>( "Assets" );
  auto sceneHierarchy = std::make_unique<kogayonon_gui::SceneHierarchyWindow>( "Scene hierarchy" );
  auto performanceWindow = std::make_unique<kogayonon_gui::PerformanceWindow>( "Performance" );
  auto entityPropertiesWindow = std::make_unique<kogayonon_gui::EntityPropertiesWindow>( "Object properties" );

  auto pImguiManager = MainRegistry::getInstance().getImGuiManager();

  pImguiManager->pushWindow( "Viewport", std::move( sceneViewport ) );
  pImguiManager->pushWindow( "Object properties", std::move( entityPropertiesWindow ) );
  pImguiManager->pushWindow( "Performance", std::move( performanceWindow ) );
  pImguiManager->pushWindow( "Scene hierarchy", std::move( sceneHierarchy ) );
  pImguiManager->pushWindow( "Assets", std::move( fileExplorerWindow ) );

  return true;
}

bool App::init()
{
  m_pWindow = std::make_shared<kogayonon_window::Window>( "kogayonon engine", 600, 400, 1, false );
  // initialize the keyboard state
  KeyboardState::initState();

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

  auto eventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  eventDispatcher->addHandler<WindowResizeEvent, &App::onWindowResize>( *this );
  eventDispatcher->addHandler<kogayonon_core::WindowCloseEvent, &App::onWindowClose>( *this );
  eventDispatcher->addHandler<kogayonon_core::ProjectLoadEvent, &App::onProjectLoad>( *this );
  eventDispatcher->addHandler<kogayonon_core::ProjectCreateEvent, &App::onProjectCreate>( *this );

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
  // ignore non-significant error/warning codes
  if ( id == 131169 || id == 131185 || id == 131218 || id == 131204 )
    return;

  // -------------------- verbose debug output --------------------
  spdlog::debug( "---------------" );
  spdlog::debug( "Debug message ({}): {}", id, message );

  const char* sourceStr = "Unknown";
  switch ( source )
  {
  case GL_DEBUG_SOURCE_API:
    sourceStr = "API";
    break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    sourceStr = "Window System";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    sourceStr = "Shader Compiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    sourceStr = "Third Party";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    sourceStr = "Application";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    sourceStr = "Other";
    break;
  }
  spdlog::debug( "Source: {}", sourceStr );

  const char* typeStr = "Unknown";
  switch ( type )
  {
  case GL_DEBUG_TYPE_ERROR:
    typeStr = "Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    typeStr = "Deprecated Behaviour";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    typeStr = "Undefined Behaviour";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    typeStr = "Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    typeStr = "Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    typeStr = "Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    typeStr = "Push Group";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    typeStr = "Pop Group";
    break;
  case GL_DEBUG_TYPE_OTHER:
    typeStr = "Other";
    break;
  }
  spdlog::debug( "Type: {}", typeStr );

  const char* severityStr = "Unknown";
  switch ( severity )
  {
  case GL_DEBUG_SEVERITY_HIGH:
    severityStr = "high";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    severityStr = "medium";
    break;
  case GL_DEBUG_SEVERITY_LOW:
    severityStr = "low";
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    severityStr = "notification";
    break;
  }
  spdlog::debug( "Severity: {}", severityStr );
  spdlog::debug( "---------------" );
}

void App::onProjectLoad( const kogayonon_core::ProjectLoadEvent& e )
{
  initGuiForProject();
  auto title = e.getPath().stem().string();
  std::string windowTitle = std::format( "kogayonon - {}", title );
  m_pWindow->setTitle( windowTitle.c_str() );

  auto& assetManager = AssetManager::getInstance();
  const auto& pTaskManager = MainRegistry::getInstance().getTaskManager();

  rapidjson::Document doc{};
  Jsoner::parseJsonFile( doc, e.getPath() );
  const auto& projectName = doc["name"].GetString();
  // seems that i don't need the path in the kproj file but we'll see
  ProjectManager::createProject( projectName, e.getPath() );

  if ( Jsoner::checkArray( doc, "scenes" ) )
  {
    const auto& scenes = doc["scenes"].GetArray();

    // now that we populated the scenes paths vector, deserialize them
    for ( const auto& scene : scenes )
    {
      const auto scenePath = std::filesystem::path{ scene["path"].GetString() };
      const auto& scene_ = std::make_shared<Scene>( scenePath.stem().string() );

      std::fstream sceneIn{ scenePath, std::ios::in | std::ios::binary };
      for ( int x = 0; x < scene["meshEntityCount"].GetInt(); x++ )
      {
        size_t meshPathSize;
        Serializer::deserialize( meshPathSize, sceneIn );

        std::string meshPath;
        meshPath.resize( meshPathSize );
        Serializer::deserialize( meshPath.data(), meshPathSize * sizeof( char ), sceneIn );

        // now rotation > scale > translation in this order
        TransformComponent tmp;
        Serializer::deserialize( tmp.rotation.x, sceneIn );
        Serializer::deserialize( tmp.rotation.y, sceneIn );
        Serializer::deserialize( tmp.rotation.z, sceneIn );

        Serializer::deserialize( tmp.scale.x, sceneIn );
        Serializer::deserialize( tmp.scale.y, sceneIn );
        Serializer::deserialize( tmp.scale.z, sceneIn );

        Serializer::deserialize( tmp.translation.x, sceneIn );
        Serializer::deserialize( tmp.translation.y, sceneIn );
        Serializer::deserialize( tmp.translation.z, sceneIn );

        auto ent = scene_->addEntity();
        ent.addComponent<TransformComponent>(
          TransformComponent{ .translation = tmp.translation, .rotation = tmp.rotation, .scale = tmp.scale } );

        // create the entity
        const auto path = std::filesystem::path{ meshPath };
        const auto& enttId = ent.getEntityId();

        pTaskManager->enqueue( [scene_, path, enttId, &assetManager]() {
          const auto mesh = assetManager.addMesh( path.stem().string(), path.string() );
          scene_->addMeshToEntity( enttId, mesh );
        } );

        std::string group;
        size_t groupSize;
        Serializer::deserialize( groupSize, sceneIn );
        group.resize( groupSize );
        Serializer::deserialize( group.data(), sizeof( char ) * groupSize, sceneIn );

        std::string name;
        size_t nameSize;
        Serializer::deserialize( nameSize, sceneIn );
        name.resize( nameSize );
        Serializer::deserialize( name.data(), sizeof( char ) * nameSize, sceneIn );

        EntityType type;
        Serializer::deserialize( type, sceneIn );

        ent.replaceComponent<IdentifierComponent>( IdentifierComponent{ .name = name, .type = type, .group = group } );
      }

      for ( int x = 0; x < scene["pointLightEntityCount"].GetInt(); x++ )
      {
        auto ent = scene_->addEntity();

        // we add a default point light component to the entity
        scene_->addPointLight( ent.getEntityId() );
        // we retrieve the newly created component
        const auto& pointLightComponent = ent.getComponent<PointLightComponent>();
        auto& light = scene_->getPointLight( pointLightComponent.pointLightIndex );

        Serializer::deserialize( light.color.x, sceneIn );
        Serializer::deserialize( light.color.y, sceneIn );
        Serializer::deserialize( light.color.z, sceneIn );
        Serializer::deserialize( light.color.w, sceneIn );

        Serializer::deserialize( light.params.x, sceneIn );
        Serializer::deserialize( light.params.y, sceneIn );
        Serializer::deserialize( light.params.z, sceneIn );
        Serializer::deserialize( light.params.w, sceneIn );

        Serializer::deserialize( light.translation.x, sceneIn );
        Serializer::deserialize( light.translation.y, sceneIn );
        Serializer::deserialize( light.translation.z, sceneIn );
        Serializer::deserialize( light.translation.w, sceneIn );

        std::string group;
        size_t groupSize;
        Serializer::deserialize( groupSize, sceneIn );
        group.resize( groupSize );
        Serializer::deserialize( group.data(), sizeof( char ) * groupSize, sceneIn );

        std::string name;
        size_t nameSize;
        Serializer::deserialize( nameSize, sceneIn );
        name.resize( nameSize );
        Serializer::deserialize( name.data(), sizeof( char ) * nameSize, sceneIn );

        EntityType type;
        Serializer::deserialize( type, sceneIn );

        ent.replaceComponent<IdentifierComponent>( IdentifierComponent{ .name = name, .type = type, .group = group } );
      }

      // doing this only once since we have only one directional light
      {
        auto ent = scene_->addEntity();

        scene_->addDirectionalLight( ent.getEntityId() );

        // we retrieve the newly created component
        auto& directionalLightComponent = ent.getComponent<DirectionalLightComponent>();
        auto& light = scene_->getDirectionalLight( directionalLightComponent.directionalLightIndex );

        Serializer::deserialize( light.direction.x, sceneIn );
        Serializer::deserialize( light.direction.y, sceneIn );
        Serializer::deserialize( light.direction.z, sceneIn );
        Serializer::deserialize( light.direction.w, sceneIn );

        Serializer::deserialize( light.diffuse.x, sceneIn );
        Serializer::deserialize( light.diffuse.y, sceneIn );
        Serializer::deserialize( light.diffuse.z, sceneIn );
        Serializer::deserialize( light.diffuse.w, sceneIn );

        Serializer::deserialize( light.specular.x, sceneIn );
        Serializer::deserialize( light.specular.y, sceneIn );
        Serializer::deserialize( light.specular.z, sceneIn );
        Serializer::deserialize( light.specular.w, sceneIn );

        Serializer::deserialize( directionalLightComponent.orthoSize, sceneIn );
        Serializer::deserialize( directionalLightComponent.nearPlane, sceneIn );
        Serializer::deserialize( directionalLightComponent.farPlane, sceneIn );
        Serializer::deserialize( directionalLightComponent.positionFactor, sceneIn );

        std::string group;
        size_t groupSize;
        Serializer::deserialize( groupSize, sceneIn );
        group.resize( groupSize );
        Serializer::deserialize( group.data(), sizeof( char ) * groupSize, sceneIn );

        std::string name;
        size_t nameSize;
        Serializer::deserialize( nameSize, sceneIn );
        name.resize( nameSize );
        Serializer::deserialize( name.data(), sizeof( char ) * nameSize, sceneIn );

        EntityType type;
        Serializer::deserialize( type, sceneIn );

        ent.replaceComponent<IdentifierComponent>( IdentifierComponent{ .name = name, .type = type, .group = group } );
      }
      if ( sceneIn )
        sceneIn.close();

      SceneManager::addScene( scene_ );

      // we do this just for testing purposes since we know that we have only one scene in the kscene file
      SceneManager::setCurrentScene( scene_->getName() );
    }
  }
}

void App::onProjectCreate( const kogayonon_core::ProjectCreateEvent& e )
{
  initGuiForProject();
  auto title = e.getPath().stem().string();
  std::string windowTitle = std::format( "kogayonon - {}", title );
  m_pWindow->setTitle( windowTitle.c_str() );

  // create the project
  ProjectManager::createProject( title, e.getPath() );

  // since this is a fresh project we add a default scene
  auto defaultScene = std::make_shared<Scene>( "Default" );

  // add a default entity
  defaultScene->addEntity();
  defaultScene->addDirectionalLight();

  // add it to the map
  SceneManager::addScene( defaultScene );

  // make it as current scene
  SceneManager::setCurrentScene( "Default" );
}

void App::onWindowClose( const kogayonon_core::WindowCloseEvent& e )
{
  // set the close flag
  m_running = false;

  if ( ProjectManager::getTitle() == "none" )
    return;

  // begin to save the work and build the json files
  rapidjson::Document projectDoc{};
  projectDoc.SetObject();
  auto& allocator = projectDoc.GetAllocator();

  const auto& scenes = SceneManager::getScenes();

  rapidjson::Value scenesArray{ rapidjson::Type::kArrayType };
  auto scenesDirPath = std::filesystem::absolute( "resources\\scenes" );
  if ( !std::filesystem::exists( scenesDirPath ) )
  {
    std::filesystem::create_directory( scenesDirPath );
  }

  for ( const auto& [name, scene] : scenes )
  {
    rapidjson::Value sceneObject{ rapidjson::Type::kObjectType };

    const auto& meshView = scene->getRegistry().getRegistry().view<MeshComponent>();
    const auto& emptyEntityView =
      scene->getRegistry().getRegistry().view<IdentifierComponent>( entt::exclude<MeshComponent, PointLightComponent> );

    // count them accurately, don't think performance drops here
    size_t meshCount = 0;
    for ( const auto& ent : meshView.each() )
      ++meshCount;

    // count them accurately, don't think performance drops here
    size_t emptyCount = 0;
    for ( const auto& ent : emptyEntityView.each() )
      ++emptyCount;

    sceneObject.AddMember( "name", rapidjson::Value{ name.c_str(), allocator }, allocator );
    sceneObject.AddMember( "directionalLightEntityCount", 1, allocator );
    sceneObject.AddMember( "meshEntityCount", meshCount, allocator );
    sceneObject.AddMember( "pointLightEntityCount", scene->getLightCount( kogayonon_resources::LightType::Point ),
                           allocator );

    auto finalPath = std::format( "{}\\{}.kscene", scenesDirPath.string(), scene->getName().c_str() );

    // this is very bad if we crash
    // TODO might just copy the file or something before crashing
    if ( std::filesystem::exists( finalPath ) )
    {
      std::filesystem::remove( finalPath );
    }

    sceneObject.AddMember( "path", rapidjson::Value{ finalPath.c_str(), allocator }, allocator );
    // use the final path to serialize entities and states
    std::fstream sceneOut{ finalPath, std::ios::out | std::ios::binary };
    std::vector<entt::entity> modelEntities;

    for ( const auto& [entity, modelComponent] : meshView.each() )
    {
      modelEntities.push_back( entity );
    }

    std::sort( modelEntities.begin(), modelEntities.end(), [&]( entt::entity a, entt::entity b ) {
      auto& meshA = scene->getRegistry().getComponent<MeshComponent>( a );
      auto& meshB = scene->getRegistry().getComponent<MeshComponent>( b );
      auto sizeA = std::filesystem::file_size( meshA.pMesh->getPath() );
      auto sizeB = std::filesystem::file_size( meshB.pMesh->getPath() );
      return sizeA < sizeB;
    } );

    for ( auto& entity : modelEntities )
    {
      Entity ent{ scene->getRegistry(), entity };
      const auto& meshComponent = ent.getComponent<MeshComponent>();
      const auto& modelPath = meshComponent.pMesh->getPath();
      const auto& transformComponent = ent.getComponent<TransformComponent>();
      const auto& identifierComponent = ent.getComponent<IdentifierComponent>();

      // we load the model and textures with the assetManager.addModel(name,path);
      size_t modelPathSize = modelPath.size();
      Serializer::serialize( modelPathSize, sceneOut );
      Serializer::serialize( modelPath.data(), modelPath.size() * sizeof( char ), sceneOut );

      Serializer::serialize( transformComponent.rotation.x, sceneOut );
      Serializer::serialize( transformComponent.rotation.y, sceneOut );
      Serializer::serialize( transformComponent.rotation.z, sceneOut );

      Serializer::serialize( transformComponent.scale.x, sceneOut );
      Serializer::serialize( transformComponent.scale.y, sceneOut );
      Serializer::serialize( transformComponent.scale.z, sceneOut );

      Serializer::serialize( transformComponent.translation.x, sceneOut );
      Serializer::serialize( transformComponent.translation.y, sceneOut );
      Serializer::serialize( transformComponent.translation.z, sceneOut );

      // now the identifier data
      const auto& group = identifierComponent.group;
      const size_t groupSize = group.size();
      Serializer::serialize( groupSize, sceneOut );
      Serializer::serialize( group.data(), sizeof( char ) * groupSize, sceneOut );

      const auto& name = identifierComponent.name;
      const size_t nameSize = name.size();
      Serializer::serialize( nameSize, sceneOut );
      Serializer::serialize( name.data(), sizeof( char ) * nameSize, sceneOut );

      const auto& type = identifierComponent.type;
      Serializer::serialize( type, sceneOut );
    }

    // serialize point lights
    for ( const auto& [entity, identifierComponent, pointLightComponent] :
          scene->getRegistry().getRegistry().view<IdentifierComponent, PointLightComponent>().each() )
    {
      const auto& light = scene->getPointLight( pointLightComponent.pointLightIndex );
      Serializer::serialize( light.color.x, sceneOut );
      Serializer::serialize( light.color.y, sceneOut );
      Serializer::serialize( light.color.z, sceneOut );
      Serializer::serialize( light.color.w, sceneOut );

      Serializer::serialize( light.params.x, sceneOut );
      Serializer::serialize( light.params.y, sceneOut );
      Serializer::serialize( light.params.z, sceneOut );
      Serializer::serialize( light.params.w, sceneOut );

      Serializer::serialize( light.translation.x, sceneOut );
      Serializer::serialize( light.translation.y, sceneOut );
      Serializer::serialize( light.translation.z, sceneOut );
      Serializer::serialize( light.translation.w, sceneOut );

      // now the identifier data
      const auto& group = identifierComponent.group;
      const size_t groupSize = group.size();
      Serializer::serialize( groupSize, sceneOut );
      Serializer::serialize( group.data(), sizeof( char ) * groupSize, sceneOut );

      const auto& name = identifierComponent.name;
      const size_t nameSize = name.size();
      Serializer::serialize( nameSize, sceneOut );
      Serializer::serialize( name.data(), sizeof( char ) * nameSize, sceneOut );

      const auto& type = identifierComponent.type;
      Serializer::serialize( type, sceneOut );
    }

    for ( const auto& [entity, identifierComponent, directionalLightComponent] :
          scene->getRegistry().getRegistry().view<IdentifierComponent, DirectionalLightComponent>().each() )
    {

      auto& light = scene->getDirectionalLight( directionalLightComponent.directionalLightIndex );

      Serializer::serialize( light.direction.x, sceneOut );
      Serializer::serialize( light.direction.y, sceneOut );
      Serializer::serialize( light.direction.z, sceneOut );
      Serializer::serialize( light.direction.w, sceneOut );

      Serializer::serialize( light.diffuse.x, sceneOut );
      Serializer::serialize( light.diffuse.y, sceneOut );
      Serializer::serialize( light.diffuse.z, sceneOut );
      Serializer::serialize( light.diffuse.w, sceneOut );

      Serializer::serialize( light.specular.x, sceneOut );
      Serializer::serialize( light.specular.y, sceneOut );
      Serializer::serialize( light.specular.z, sceneOut );
      Serializer::serialize( light.specular.w, sceneOut );

      Serializer::serialize( directionalLightComponent.orthoSize, sceneOut );
      Serializer::serialize( directionalLightComponent.nearPlane, sceneOut );
      Serializer::serialize( directionalLightComponent.farPlane, sceneOut );
      Serializer::serialize( directionalLightComponent.positionFactor, sceneOut );

      // now the identifier data
      const auto& group = identifierComponent.group;
      const size_t groupSize = group.size();
      Serializer::serialize( groupSize, sceneOut );
      Serializer::serialize( group.data(), sizeof( char ) * groupSize, sceneOut );

      const auto& name = identifierComponent.name;
      const size_t nameSize = name.size();
      Serializer::serialize( nameSize, sceneOut );
      Serializer::serialize( name.data(), sizeof( char ) * nameSize, sceneOut );

      const auto& type = identifierComponent.type;
      Serializer::serialize( type, sceneOut );
    }

    if ( sceneOut )
      sceneOut.close();

    scenesArray.PushBack( sceneObject, allocator );
  }

  auto projectPath = ProjectManager::getPath();

  projectDoc.AddMember( "name", rapidjson::Value{ ProjectManager::getTitle().c_str(), allocator }, allocator );
  projectDoc.AddMember( "path", rapidjson::Value{ projectPath.string().c_str(), allocator }, allocator );

  projectDoc.AddMember( "scenes", scenesArray, allocator );
  Jsoner::writeJsonFile( projectDoc, projectPath );
}
} // namespace kogayonon_app