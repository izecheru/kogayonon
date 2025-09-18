#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "app/app.hpp"

int main( int argc, char* argv[] )
{
  kogayonon_app::App app;
  app.run();
  return 0;
}