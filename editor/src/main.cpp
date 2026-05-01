#include <SDL2/SDL.h>
#include <Windows.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include "editor/editor.hpp"
#include "utilities/utils/utils.hpp"

#ifdef _DEBUG
#ifdef _WIN32
int SDL_main( int argc, char** argv )
{
  editor::Editor editor;
  try
  {
    editor.run();
    editor.cleanup();
  }
  catch ( std::exception& e )
  {
    KOGAYONON_ERR( e.what() );
    editor.cleanup();
  }
  return 0;
}
#endif
#else
#ifdef _WIN32
static int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow )
{
  try
  {
    editor::Editor editor;
    editor.run();
    editor.cleanup();
  }
  catch ( std::exception& e )
  {
    KOGAYONON_ERR( e.what() );
  }
  return 0;
}
#endif
#endif