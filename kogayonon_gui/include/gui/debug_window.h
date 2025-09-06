#pragma once
#include <mutex>
#include <vector>
#include "gui/imgui_window.h"
#include "logger/logger.h"

using namespace kogayonon_logger;

namespace kogayonon_gui {
class DebugConsoleWindow : public ImGuiWindow
{
  public:
    DebugConsoleWindow( std::string name ) : ImGuiWindow( name ) {}

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
    inline ImVec4 logTypeToColor( LogType type )
    {
        switch ( type )
        {
        case LogType::DEBUG:
            return ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ); // Gray
        case LogType::INFO:
            return ImVec4( 0.0f, 0.7f, 1.0f, 1.0f ); // Cyan / Blue
        case LogType::WARN:
            return ImVec4( 1.0f, 0.8f, 0.0f, 1.0f ); // Yellow/Orange
        case LogType::ERROR:
            return ImVec4( 1.0f, 0.2f, 0.2f, 1.0f ); // Red
        case LogType::CRITICAL:
            return ImVec4( 0.8f, 0.0f, 0.0f, 1.0f ); // Darker Red
        default:
            return ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ); // White (fallback)
        }
    }

  private:
    bool m_auto_scroll = true;
    static inline std::mutex m_mutex;
    static inline std::vector<std::string> m_messages;
};
} // namespace kogayonon_gui