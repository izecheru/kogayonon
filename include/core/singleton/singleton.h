#pragma once

namespace kogayonon
{
  template <typename T>
  class Singleton
  {
  public:
    static T& getInstance()
    {
      static T instance;
      return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

  protected:
    Singleton() { }
    ~Singleton() { }
  };
}
