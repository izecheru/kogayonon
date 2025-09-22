#pragma once
#include <Windows.h>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace kogayonon_utilities
{
class DirectoryWatcher
{
  using Callback = std::function<void( std::string, std::string )>;

public:
  DirectoryWatcher( std::filesystem::path root );
  ~DirectoryWatcher();

  void run( std::filesystem::path root );

  void setCommand( const std::string& commandName, Callback callback )
  {
    m_commands.emplace( commandName, callback );
  }

private:

  // string for name of command and callback for the emit event from event dispatcher
  std::unordered_map<std::string, Callback> m_commands;
  std::thread m_watcherThread;
  std::mutex m_mutex;
  std::filesystem::path m_root;
  HANDLE m_dirHandle{ nullptr };
  HANDLE m_shutdownHandle{ nullptr };
  OVERLAPPED m_overlapped{};
  std::atomic_bool m_stop = false;
};
} // namespace kogayonon_utilities