#pragma once
#include <memory>
#include <mutex>

template <typename T>
class Singleton
{
public:
  // Get the singleton instance
  static T& getInstance() {
    static T instance;
    return instance;
  }

  // Deleted copy constructor and assignment operator to prevent copying
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;

protected:
  // Protected constructor and destructor to ensure only derived classes can use them
  Singleton() {}
  ~Singleton() {}
};