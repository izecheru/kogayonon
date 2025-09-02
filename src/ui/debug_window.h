#pragma once
#include <mutex>
#include <vector>

#include "imgui_window.h"

namespace kogayonon
{
class DebugConsoleWindow : public ImGuiWindow
{
public:
  DebugConsoleWindow(std::string name) : ImGuiWindow(std::move(name)) {}

  void clearLogs();

  template <typename... Args>
  static void log(const Args&... args)
  {
    std::stringstream str_stream{};

    (str_stream << ... << args);
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_messages.push_back(str_stream.str());
    }
  }

  void draw() override;

private:
  bool m_auto_scroll = true;
  static inline std::mutex m_mutex;
  static inline std::vector<std::string> m_messages;
};
} // namespace kogayonon