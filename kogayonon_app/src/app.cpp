#include "app/app.hpp"
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
#include "rendering/renderer.hpp"
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
  glDebugMessageCallback( glDebugCallback, nullptr );
#endif
  glEnable( GL_DEPTH_TEST );
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
  shaderManager->pushShader( "resources/shaders/white_vertex.glsl", "resources/shaders/white_fragment.glsl", "white" );
  shaderManager->pushShader( "resources/shaders/picking_vertex.glsl", "resources/shaders/picking_fragment.glsl",
                             "picking" );
  shaderManager->pushShader( "resources/shaders/depth_vert.glsl", "resources/shaders/depth_frag.glsl", "depth" );
  shaderManager->pushShader( "resources/shaders/depth_vert.glsl", "resources/shaders/depth_debug_frag.glsl",
                             "depthDebug" );

  shaderManager->compileShaders();

  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::ShaderManager>>( std::move( shaderManager ) );

  // init asset manager
  auto assetManager = std::make_shared<kogayonon_utilities::AssetManager>();
  assert( assetManager && "could not initialise asset manager" );

  assetManager->addTexture( "play.png" );
  assetManager->addTexture( "stop.png" );
  assetManager->addTexture( "file.png" );
  assetManager->addTexture( "folder.png" );
  assetManager->addTexture( "default.png" );
  assetManager->addTexture( "3d-cube.png" );
  assetManager->addTexture( "logo.png" );
  assetManager->addTexture( "render_mode_icon.png" );

  assetManager->addMesh( "default", "resources/models/Cube.gltf" );

  mainRegistry.addToContext<std::shared_ptr<kogayonon_utilities::AssetManager>>( std::move( assetManager ) );

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
  m_pWindow->maximize();

  // deserialize data and read scene and entity stuff
  // ---------

  const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();

  // textures for imgui buttons or windows and what not
  auto playTexture = pAssetManager->getTextureByName( "play.png" ).lock()->getTextureId();
  auto stopTexture = pAssetManager->getTextureByName( "stop.png" ).lock()->getTextureId();
  auto fileTexture = pAssetManager->getTextureByName( "file.png" ).lock()->getTextureId();
  auto folderTexture = pAssetManager->getTextureByName( "folder.png" ).lock()->getTextureId();

  auto sceneViewport = std::make_unique<kogayonon_gui::SceneViewportWindow>( m_pWindow->getWindow(), "Viewport",
                                                                             playTexture, stopTexture );
  auto fileExplorerWindow = std::make_unique<kogayonon_gui::FileExplorerWindow>( "Assets", folderTexture, fileTexture );
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
  // i care about only medium to high severity for now
  if ( severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM )
  {
    spdlog::error( "[OpenGL Debug] Severity: {} | Type: {} | Source: {} | ID: {} | Message: {}",
                   glSeverityToStr( severity ), glTypeToStr( type ), glSourceToStr( source ), id, message );
  }
}

void App::onProjectLoad( const kogayonon_core::ProjectLoadEvent& e )
{
  initGuiForProject();
  auto title = e.getPath().stem().string();
  std::string windowTitle = std::format( "kogayonon - {}", title );
  m_pWindow->setTitle( windowTitle.c_str() );

  const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();
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
      // TODO deserialize entities here
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

        pTaskManager->enqueue( [scene_, path, enttId, pAssetManager]() {
          const auto mesh = pAssetManager->addMesh( path.stem().string(), path.string() );
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

        kogayonon_resources::PointLight tempLight{};
        Serializer::deserialize( tempLight.color.x, sceneIn );
        Serializer::deserialize( tempLight.color.y, sceneIn );
        Serializer::deserialize( tempLight.color.z, sceneIn );
        Serializer::deserialize( tempLight.color.w, sceneIn );

        Serializer::deserialize( tempLight.params.x, sceneIn );
        Serializer::deserialize( tempLight.params.y, sceneIn );
        Serializer::deserialize( tempLight.params.z, sceneIn );
        Serializer::deserialize( tempLight.params.w, sceneIn );

        Serializer::deserialize( tempLight.translation.x, sceneIn );
        Serializer::deserialize( tempLight.translation.y, sceneIn );
        Serializer::deserialize( tempLight.translation.z, sceneIn );
        Serializer::deserialize( tempLight.translation.w, sceneIn );

        light = tempLight;

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

      // we load the model and textures with the assetManager->addModel(name,path);
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