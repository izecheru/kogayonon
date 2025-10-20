#include <SDL2/SDL.h>
#include <Windows.h>

#include "app/app.hpp"

#ifndef NDEBUG
#ifdef _WIN32
int SDL_main( int argc, char** argv )
{
  kogayonon_app::App app;
  app.run();
  return 0;
}
#endif
#else
#ifdef _WIN32
static int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow )
{
  kogayonon_app::App app;
  app.run();
  return 0;
}
#endif
#endif