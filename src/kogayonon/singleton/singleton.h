#pragma once
#include <mutex>

class Singleton {
private:
  static Singleton* instance_ptr;
  static std::mutex mtx;
  Singleton() {}
public:
  Singleton(const Singleton& obj) = delete;
  void operator=(const Singleton& obj) = delete;
  inline static Singleton* getInstance();
};

