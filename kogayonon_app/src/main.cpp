#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <Windows.h>

#include "app/app.hpp"

int main( int argc, char* argv[] )
{
#ifndef NDEBUG
#ifdef _WIN32
  ShowWindow( GetConsoleWindow(), SW_SHOW );
#endif
#else
#ifdef _WIN32
  ShowWindow( GetConsoleWindow(), SW_HIDE );
#endif
#endif
  kogayonon_app::App app;
  app.run();
  return 0;
}