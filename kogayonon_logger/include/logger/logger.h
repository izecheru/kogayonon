#pragma once
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>

namespace kogayonon_logger {
enum class LogType
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

class Logger
{
  public:
    using LogCallback = std::function<void( const std::string& )>;

    Logger() = delete;

    static void logWorker();
    static void shutdown();

    static void inline addCallback( LogCallback callback )
    {
        std::unique_lock lock( m_mutex );
        m_callbacks.push_back( callback );
    }

    template <typename... Args>
    static void log( LogType type, Args&&... args )
    {
        time_t log_time = time( nullptr );
        struct tm local_time;
        localtime_s( &local_time, &log_time );

        std::unique_lock lock( m_mutex );
        m_str_stream << "[" << std::put_time( &local_time, "%H:%M:%S" ) << "]";
        switch ( type )
        {
        case LogType::INFO:
            m_str_stream << "[info] ";
            break;
        case LogType::DEBUG:
            m_str_stream << "[debug] ";
            break;
        case LogType::WARN:
            m_str_stream << "[warning] ";
            break;
        case LogType::ERROR:
            m_str_stream << "[error]";
            break;
        case LogType::CRITICAL:
            m_str_stream << "[critical] ";
            break;
        }

        ( m_str_stream << ... << args );
        m_queued_logs.push( m_str_stream.str() );

        m_cv.notify_all();
        for ( auto& callback : m_callbacks )
        {
            callback( m_str_stream.str() );
        }
        m_str_stream.str( "" );
        m_str_stream.clear();
    }

    template <typename... Args>
    static void error( Args&&... args )
    {
        log( LogType::ERROR, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void critical( Args&&... args )
    {
        log( LogType::CRITICAL, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void info( Args&&... args )
    {
        log( LogType::INFO, std::forward<Args>( args )... );
    }

    static void initialize( const std::string& log_path );

  private:
    static inline std::queue<std::string> m_queued_logs;
    static inline std::thread m_worker_thread;
    static inline std::condition_variable m_cv;
    static inline std::vector<LogCallback> m_callbacks;

    static inline std::stringstream m_str_stream;
    static inline std::mutex m_mutex;
    static inline std::ofstream m_out;
};
} // namespace kogayonon_logger
