#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <glfw3.h>
#include <trigonometric.hpp>
#include <iostream>
#include "src/kogayonon/app/app.h"

int main(void) {
  kogayonon::App app;
  app.mainLoop();
  return 0;
}