#include <SDL2/SDL.h>
#include <Windows.h>
#include <iostream>

#include "editor/editor.hpp"

#ifdef _DEBUG
#ifdef _WIN32
int SDL_main( int argc, char** argv )
{
  try
  {
    editor::Editor editor;
    editor.run();
  }
  catch ( std::exception& e )
  {
    std::cout << '\n' << e.what();
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
  }
  catch ( std::exception& e )
  {
    std::cout << '\n' << e.what();
  }
  return 0;
}
#endif
#endif