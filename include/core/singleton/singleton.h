#pragma once
#include <memory>
#include <mutex>

namespace kogayonon
{
  template <typename T>
  class Singleton
  {
  public:
    static T& getInstance() {
      static T instance;
      return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

  protected:
    Singleton() {}
    ~Singleton() {}
  };
}
