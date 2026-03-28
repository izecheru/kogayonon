#include "editor/editor.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <iostream>
#include <memory>
#include <rapidjson/istreamwrapper.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "core/asset_manager/asset_manager.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "utilities/config_manager/config_manager.hpp"
#include "window/window.hpp"

namespace editor
{
Editor::Editor()
{
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

  utilities::EditorConfigManager::initConfig();

  init();
}

Editor::~Editor()
{
  cleanup();
}

void Editor::cleanup() const
{
  spdlog::info( "Closing editor and cleaning up" );
}

void Editor::pollEvents()
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
        // WindowResizeEvent windowResizeEvent{ newWidth, newHeight };
        // pEventDispatcher->dispatchEvent( windowResizeEvent );
      }
      break;
    }
    case SDL_QUIT: {
      // pEventDispatcher->dispatchEvent( WindowCloseEvent{} );
      m_running = false;
      break;
    }
    case SDL_KEYDOWN: {
      // KeyboardState::updateState();
      // auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );

      // KeyPressedEvent keyPressEvent{ scanCode, KeyScanCode::None, 0 };
      // if ( KeyboardState::getKeyState( KeyScanCode::LeftControl ) )
      //{
      //   keyPressEvent.setKeyModifier( KeyScanCode::LeftControl );
      // }

      // if ( KeyboardState::getKeyState( KeyScanCode::LeftShift ) )
      //{
      //   keyPressEvent.setKeyModifier( KeyScanCode::LeftShift );
      // }

      // pEventDispatcher->dispatchEvent( keyPressEvent );
      break;
    }
    case SDL_KEYUP: {
      // KeyboardState::updateState();
      // auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );
      // KeyReleasedEvent keyReleaseEvent{ scanCode, KeyScanCode::None };
      // pEventDispatcher->dispatchEvent( keyReleaseEvent );
      // break;
    }
    case SDL_MOUSEMOTION: {
      double x = e.motion.x;
      double y = e.motion.y;
      double xRel = e.motion.xrel;
      double yRel = e.motion.yrel;
      // MouseMovedEvent mouseMovedEvent{ x, y, xRel, yRel };
      // pEventDispatcher->dispatchEvent( mouseMovedEvent );
      break;
    }
    case SDL_MOUSEWHEEL: {
      double xOff = e.wheel.x;
      double yOff = e.wheel.y;
      // MouseScrolledEvent mouseScrolled{ xOff, yOff };
      // pEventDispatcher->dispatchEvent( mouseScrolled );
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      // UINT32 buttonState = SDL_GetMouseState( NULL, NULL );
      // if ( buttonState & SDL_BUTTON( SDL_BUTTON_MIDDLE ) )
      //{
      //   MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_MIDDLE ),
      //                                   static_cast<int>( MouseAction::Press ),
      //                                   static_cast<int>( MouseModifier::None ) };
      //   pEventDispatcher->dispatchEvent( mouseClicked );
      // }
      // if ( buttonState & SDL_BUTTON( SDL_BUTTON_LEFT ) )
      //{
      //   MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_LEFT ),
      //                                   static_cast<int>( MouseAction::Press ),
      //                                   static_cast<int>( MouseModifier::None ) };
      //   pEventDispatcher->dispatchEvent( mouseClicked );
      // }
      // if ( buttonState & SDL_BUTTON( SDL_BUTTON_RIGHT ) )
      //{
      //   MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_RIGHT ),
      //                                   static_cast<int>( MouseAction::Press ),
      //                                   static_cast<int>( MouseModifier::None ) };
      //   pEventDispatcher->dispatchEvent( mouseClicked );
      // }
      break;
    }
    default:
      break;
    }
  }
}

void Editor::run()
{
  while ( m_running )
  {
    pollEvents();
    m_pSwapchain->beginRendering();
    m_pImguiRenderer->render();
    auto& cmd = m_pSwapchain->getCurrentCommandBuffer();
    m_pImguiRenderer->present( cmd );
    m_pSwapchain->endRendering();
    m_pSwapchain->presentFrame();
  }
}

bool Editor::initSDL()
{
  if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ) != 0 )
  {
    spdlog::error( "SDL_Init Error: {}", SDL_GetError() );
    throw std::runtime_error( "SDL_Init failed" );
  }

  if ( SDL_Vulkan_LoadLibrary( nullptr ) != 0 )
  {
    spdlog::error( "SDL Vulkan load failed: {}", SDL_GetError() );
    throw std::runtime_error( "could not load lib vulkan" );
  }

  return true;
}

bool Editor::initVulkan()
{
  m_pDevice = std::make_shared<graphics::VulkanDevice>( m_pWindow->getWindow() );
  m_pSwapchain = std::make_shared<graphics::VulkanSwapchain>( m_pDevice.get(), m_pWindow->getWindow() );
  auto& assetManager = core::AssetManager::get();
  assetManager.setDevice( m_pDevice.get() );
  assetManager.setSwapchain( m_pSwapchain.get() );

  return true;
}

bool Editor::initImgui()
{
  m_pImguiRenderer =
    std::make_shared<gui::VulkanImguiRenderer>( m_pWindow->getWindow(), m_pDevice.get(), m_pSwapchain.get() );

  return true;
}

bool Editor::initMainWindow()
{
  auto& cfg = utilities::EditorConfigManager::getConfig();

  m_pWindow = std::make_shared<window::Window>( "kogayonon engine", cfg.width, cfg.height, false, cfg.maximized );
  m_pWindow->setBordered( true );
  m_pWindow->setResizable( true );
  return true;
}

bool Editor::init()
{

  if ( !initSDL() )
  {
    throw std::runtime_error( "sdl could not be initialized" );
  }

  if ( !initMainWindow() )
  {
    throw std::runtime_error( "could not initialize main window" );
  }

  if ( !initVulkan() )
  {
    throw std::runtime_error( "vulkan could not be initialized" );
  }

  if ( !initImgui() )
  {
    throw std::runtime_error( "vulkan could not be initialized" );
  }

  m_running = true;
  return true;
}

} // namespace editor