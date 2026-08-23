#include "editor/editor.hpp"
#include "utilities/utils/utils.hpp"
#include <SDL2/SDL.h>
#include <Windows.h>
#include <iostream>

#ifdef _DEBUG
#ifdef _WIN32
int SDL_main( int argc, char** argv )
{
  editor::Editor editor{};
  try
  {
    editor.run();
  }
  catch ( std::exception& e )
  {
    K_ERROR( "{}", e.what() );
  }
  editor.cleanup();
  return 0;
}
#endif
#else
#ifdef _WIN32
static int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow )
{
  editor::Editor editor;
  try
  {
    editor.run();
  }
  catch ( std::exception& e )
  {
    printf_s( "%s", e.what() );
  }
  editor.cleanup();
  return 0;
}
#endif
#endif