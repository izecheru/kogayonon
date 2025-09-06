#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "app/app.h"

int main( int argc, char* argv[] )
{
    kogayonon_app::App app;
    app.run();
    return 0;
}