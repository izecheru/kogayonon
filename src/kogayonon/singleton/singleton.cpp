#include "singleton.h"

Singleton* Singleton::getInstance() {
  if (instance_ptr == nullptr) {
    std::lock_guard<std::mutex> lock(mtx);
    if (instance_ptr == nullptr) {
      instance_ptr = new Singleton();
    }
  }
  return instance_ptr;
}
