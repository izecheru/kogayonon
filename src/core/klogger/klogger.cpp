#include "core/klogger/klogger.h"
#include <thread>

namespace kogayonon
{
  std::stringstream KLogger::m_str_stream;
  std::mutex KLogger::m_mutex;
  std::ofstream KLogger::m_out;
  std::queue<std::string> KLogger::m_queued_logs;
  std::condition_variable KLogger::m_cv;
  std::thread KLogger::m_worker_thread;

  void KLogger::initialize(const std::string& log_path)
  {
    {
      std::unique_lock lock(m_mutex);
      m_out.open(log_path, std::ios::out);
    }
    m_worker_thread = std::thread(&logWorker);
  }

  void KLogger::shutdown()
  {
    {
      std::unique_lock lock(m_mutex);
      m_queued_logs.emplace("Shutting down logger...");
    }
    m_cv.notify_all();
    if(m_worker_thread.joinable())
    {
      m_worker_thread.join();
    }
    if(m_out.is_open())
    {
      m_out.close();
    }
  }

  void KLogger::logWorker()
  {
    while(true)
    {
      std::string log_message;
      {
        std::unique_lock lock(m_mutex);
        m_cv.wait(lock, []
          {
            return !m_queued_logs.empty();
          });
        log_message = m_queued_logs.front();
        m_queued_logs.pop();
      }

      if(log_message == "Shutting down logger...")
      {
        break;
      }

      if(m_out.is_open())
      {
        m_out << log_message << '\n';
      }
    }
  }
}