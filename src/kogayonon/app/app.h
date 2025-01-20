#pragma once
#include "../event/applicationEvent.h"
namespace kogayonon
{
  class App
  {
  public:
    void mainLoop();
    void onEvent(Event& event);
    void onWindowClose();
    void onWindowResize(WindowResizeEvent& event);
  };
}
