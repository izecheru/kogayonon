#pragma once
#include <mutex>

namespace kogayonon
{
  template <typename T>
  class Singleton
  {
  public:
    static T* getInstance()
    {
      static T instance;  // Thread-safe since C++11, no leaks
      return &instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

  protected:
    Singleton() = default;
    ~Singleton() = default;
  };
}
