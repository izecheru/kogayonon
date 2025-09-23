#include "utilities/directory_watcher/directory_watcher.hpp"
#include <codecvt>
#include <Windows.h>
#include <assert.h>
#include <locale>
#include <spdlog/spdlog.h>

namespace kogayonon_utilities
{
DirectoryWatcher::DirectoryWatcher( std::filesystem::path root )
    : m_root( root )
{
  m_shutdownHandle = CreateEvent( nullptr, TRUE, FALSE, nullptr );
  m_overlapped.hEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );

  // Open handle to directory
  m_dirHandle =
    CreateFileW( m_root.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                 OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr );

  if ( m_dirHandle == INVALID_HANDLE_VALUE )
  {
    throw std::runtime_error( "Failed to open directory handle" );
  }

  // Launch watcher thread
  m_watcherThread = std::thread( [this]() { run( m_root ); } );
}

DirectoryWatcher::~DirectoryWatcher()
{
  if ( !m_stop.exchange( true ) )
  {
    SetEvent( m_shutdownHandle );
    if ( m_watcherThread.joinable() )
    {
      m_watcherThread.join();
    }
  }
  if ( m_dirHandle && m_dirHandle != INVALID_HANDLE_VALUE )
  {
    CloseHandle( m_dirHandle );
  }
  if ( m_shutdownHandle )
  {
    CloseHandle( m_shutdownHandle );
  }
  if ( m_overlapped.hEvent )
  {
    CloseHandle( m_overlapped.hEvent );
  }
}

void DirectoryWatcher::run( std::filesystem::path root )
{
  uint8_t buffer[8192];

  while ( !m_stop )
  {
    BOOL success =
      ReadDirectoryChangesW( m_dirHandle, buffer, sizeof( buffer ),
                             TRUE, // recursive to check all the underlying dirs
                             FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                             nullptr, &m_overlapped, nullptr );

    if ( !success )
    {
      spdlog::error( "ReadDirectoryChangesW failed" );
      break;
    }

    HANDLE handles[2] = { m_overlapped.hEvent, m_shutdownHandle };

    DWORD waitResult = WaitForMultipleObjects( 2, handles, FALSE, INFINITE );

    if ( waitResult == WAIT_OBJECT_0 + 1 )
    {
      // shutdown requested
      break;
    }

    if ( waitResult == WAIT_OBJECT_0 )
    {
      DWORD bytesTransferred = 0;
      if ( !GetOverlappedResult( m_dirHandle, &m_overlapped, &bytesTransferred, FALSE ) )
      {
        spdlog::error( "GetOverlappedResult failed" );
        continue;
      }

      FILE_NOTIFY_INFORMATION* event = reinterpret_cast<FILE_NOTIFY_INFORMATION*>( buffer );

      for ( ;; )
      {
        DWORD nameLen = event->FileNameLength / sizeof( wchar_t );
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        std::wstring wFilename( event->FileName, nameLen );
        std::filesystem::path relativePath = conv.to_bytes( wFilename ); // path relative to watched folder

        // Full path relative to project root
        std::filesystem::path fullPath = m_root / relativePath;

        // Get name without extension
        std::string nameOnly = fullPath.stem().string();

        // Get path starting from "resources/"
        std::filesystem::path pathFromResources;
        auto it = std::find( fullPath.begin(), fullPath.end(), "resources" );
        if ( it != fullPath.end() )
        {
          pathFromResources = std::filesystem::path( "" );
          for ( ; it != fullPath.end(); ++it )
            pathFromResources /= *it;
        }
        switch ( event->Action )
        {
        case FILE_ACTION_ADDED:
          m_commands.at( "fileCreated" )( pathFromResources.string(), nameOnly );
          break;
        case FILE_ACTION_REMOVED:
          m_commands.at( "fileDeleted" )( pathFromResources.string(), nameOnly );
          break;
        case FILE_ACTION_MODIFIED:

          m_commands.at( "fileModified" )( pathFromResources.string(), nameOnly );
          break;
        case FILE_ACTION_RENAMED_OLD_NAME:

          // m_commands.at( "fileRenamedOld" )( m_root, filename );
          break;
        case FILE_ACTION_RENAMED_NEW_NAME:
          m_commands.at( "fileRenamedNew" )( pathFromResources.string(), nameOnly );
          break;
        default:
          spdlog::info( "Unknown action!" );
          break;
        }

        if ( event->NextEntryOffset )
        {
          event =
            reinterpret_cast<FILE_NOTIFY_INFORMATION*>( reinterpret_cast<uint8_t*>( event ) + event->NextEntryOffset );
        }
        else
        {
          break;
        }
      }
    }
  }
}
} // namespace kogayonon_utilities