#pragma once
#include <Windows.h>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace kogayonon_core
{
enum FileEventType;
}

namespace kogayonon_utilities
{
class DirectoryWatcher
{
  using FileEventCallback = std::function<void( std::string, std::string, kogayonon_core::FileEventType type )>;

public:
  DirectoryWatcher( std::filesystem::path root );
  ~DirectoryWatcher();

  /**
   * @brief Starts the directory watcher in the path pointed by the root param
   * @param root
   */
  void run( std::filesystem::path root );

  /**
   * @brief Sets the callback func
   * @param callback
   */
  void setCallback( FileEventCallback callback )
  {
    m_eventCallbackFunc = callback;
  }

private:
  FileEventCallback m_eventCallbackFunc;
  std::thread m_watcherThread;
  std::mutex m_mutex;
  std::filesystem::path m_root;
  HANDLE m_dirHandle{ nullptr };
  HANDLE m_shutdownHandle{ nullptr };
  OVERLAPPED m_overlapped{};
  std::atomic_bool m_stop = false;
};
} // namespace kogayonon_utilities