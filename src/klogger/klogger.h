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
    KLogger(const std::string& path)
    {
      initialize(path);
    }

    ~KLogger()
    {
      shutdown();
    }

    void logWorker();
    void shutdown();

    template <typename... Args>
    void log(LogType type, const Args&... args)
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
    void initialize(const std::string& log_path);

    std::queue<std::string> m_queued_logs;
    std::thread m_worker_thread;
    std::condition_variable m_cv;

    std::stringstream m_str_stream;
    std::mutex m_mutex;
    std::ofstream m_out;
  };
} // namespace kogayonon
