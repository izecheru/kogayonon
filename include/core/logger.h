#pragma once
#include <string>
#include <sstream>
#include <iostream>

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

    template <typename T>
    static void appendToStream(std::stringstream& stream, const T& value)
    {
      stream << value;
    }

    template <typename T, typename... Args>
    static void appendToStream(std::stringstream& stream, const T& value, const Args&... args)
    {
      stream << value;
      appendToStream(stream, args...);
    }

  public:
    template <typename... Args>
    static void logError(const Args&... args)
    {
      red();
      std::stringstream stream;
      appendToStream(stream, args...);
      std::cerr << "[ERROR]: " << stream.str() << std::endl;
      reset();
    }

    template <typename... Args>
    static void logInfo(const Args&... args)
    {
      green();
      std::stringstream stream;
      appendToStream(stream, args...);
      std::cout << "[INFO]: " << stream.str() << std::endl;
      reset();
    }
  };
}
