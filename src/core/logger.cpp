#include "core/logger.h"

namespace kogayonon
{
  void Logger::green() {
    printf("\033[0;32m");
  }

  void Logger::red() {
    printf("\033[1;31m");
  }

  void Logger::reset() {
    printf("\033[0m");
  }
}
