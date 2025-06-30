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
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_pInstance == nullptr)
        {
          m_pInstance = new T();
        }
      }
      return m_pInstance;
    }

    Singleton(Singleton& other) = delete;
    Singleton& operator=(Singleton&) = delete;

  private:
    static inline std::mutex m_mutex;
    static inline T* m_pInstance;

  protected:
    Singleton() = default;
    ~Singleton() = default;
  };
}
