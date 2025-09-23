#pragma once
#include <mutex>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include <vector>
#include "gui/imgui_window.hpp"

namespace kogayonon_gui
{
class DebugConsoleWindow : public ImGuiWindow
{
public:
  DebugConsoleWindow( std::string name )
      : ImGuiWindow( std::move( name ) )
  {
  }

  void clearLogs();

  template <typename... Args>
  static void log( const Args&... args )
  {
    std::stringstream str_stream{};

    ( str_stream << ... << args );
    {
      std::unique_lock<std::mutex> lock( m_mutex );
      m_messages.push_back( str_stream.str() );
    }
  }

  void draw() override;

private:
  bool m_auto_scroll = true;
  static inline std::mutex m_mutex;
  static inline std::vector<std::string> m_messages;
};

template <typename Mutex>
class DeferredImGuiSink : public spdlog::sinks::base_sink<Mutex>
{
public:
  void setWindow( DebugConsoleWindow* window )
  {
    m_window = window;
  }

protected:
  void sink_it_( const spdlog::details::log_msg& msg ) override
  {
    if ( !m_window )
      return;

    spdlog::memory_buf_t formatted;
    this->formatter_->format( msg, formatted );

    m_window->log( std::string( formatted.data(), formatted.size() ) );
  }

  void flush_() override
  {
  }

private:
  DebugConsoleWindow* m_window;
};
} // namespace kogayonon_gui