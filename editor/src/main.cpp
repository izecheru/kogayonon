#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <Windows.h>
#include "editor/editor.hpp"
#include "utilities/utils/utils.hpp"

int main( int argc, char** argv )
{
    editor::Editor editor{};
    try
    {
        editor.run();
    }
    catch ( std::exception& e )
    {
        KERROR( "{}", e.what() );
    }
    editor.cleanup();
    return 0;
}

int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    return main( __argc, __argv );
}
