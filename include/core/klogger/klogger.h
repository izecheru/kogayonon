#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>

namespace kogayonon
{
  enum class LogType
  {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
  };

  class KLogger
  {
  public:
    // Delete constructor and destructor to prevent instantiation
    KLogger()  = delete;
    ~KLogger() = delete;

    static void logWorker();
    static void shutdown();

    static void initialize(const std::string& log_path);
    template <typename... Args> static void log(LogType type, const Args&... args)
    {
      time_t log_time = time(nullptr);
      struct tm local_time;
      localtime_s(&local_time, &log_time);

      std::unique_lock lock(m_mutex);
      m_str_stream << "[" << std::put_time(&local_time, "%H:%M:%S") << "]";
      switch (type)
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

      (m_str_stream << ... << args);
      m_queued_logs.push(m_str_stream.str());

      m_cv.notify_all();
      m_str_stream.str("");
      m_str_stream.clear();
    }

  private:
    static std::queue<std::string> m_queued_logs;
    static std::thread m_worker_thread;
    static std::condition_variable m_cv;

    static std::stringstream m_str_stream;
    static std::mutex m_mutex;
    static std::ofstream m_out;
  };
} // namespace kogayonon
