#pragma once
#include "precompiled/pch.hpp"
#include <Windows.h>
#include <shellapi.h>
#include "core/event/file_events.hpp"

namespace utilities
{
class DirectoryWatcher
{
    using FileEventCallback = std::function<void( std::string, std::string, core::FileEventType type )>;

  public:
    explicit DirectoryWatcher( std::filesystem::path root );
    ~DirectoryWatcher();

    /**
     * @brief Starts the directory watcher in the path pointed by the root param
     * @param root
     */
    auto run( std::filesystem::path root ) -> void;

    /**
     * @brief Sets the callback func
     * @param callback
     */
    inline auto setCallback( FileEventCallback callback ) -> void
    {
        m_eventCallbackFunc = callback;
    }

  private:
    FileEventCallback m_eventCallbackFunc;
    std::jthread m_watcherThread;
    std::mutex m_mutex;
    std::filesystem::path m_root;
    HANDLE m_dirHandle{ nullptr };
    HANDLE m_shutdownHandle{ nullptr };
    OVERLAPPED m_overlapped{};
    std::atomic_bool m_stop{ false };
};
} // namespace utilities
