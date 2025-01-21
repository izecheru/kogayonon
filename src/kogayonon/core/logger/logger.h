#pragma once
#include <string>

namespace kogayonon
{
  class Logger
  {
  public:
    Logger() = delete;

  private:
    static void red();
    static void green();
    static void reset();

  public:
    static void logError(std::string text);
    static void logInfo(std::string text);
  };
}
