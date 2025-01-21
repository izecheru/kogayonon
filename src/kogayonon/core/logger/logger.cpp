#include "logger.h"

namespace kogayonon
{
  void Logger::green()
  {
    printf("\033[0;32m");
  }

  void Logger::red()
  {
    printf("\033[1;31m");
  }

  void Logger::reset()
  {
    printf("\033[0m");
  }

  void Logger::logError(std::string text)
  {
    red();
    printf_s("%s\n", text.c_str());
    reset();
  }
  void Logger::logInfo(std::string text)
  {
    green();
    printf_s("%s\n", text.c_str());
    reset();
  }
}
