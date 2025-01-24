#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <glm/trigonometric.hpp>
#include <iostream>
#include "app/app.h"

#undef main
int main(void) {
  App app;
  app.run();
  return 0;
}