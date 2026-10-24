#pragma once
#include "gui/imgui_windows/imgui_base.hpp"

namespace graphics
{
class VulkanDevice;
}

namespace gui
{
struct EngineDataSpec
{
    std::unordered_map<std::string, ImFont*>* fonts;
};

class EngineDataWindow : public ImGuiWindow
{
  public:
    explicit EngineDataWindow( const std::string& name, const EngineDataSpec& spec, graphics::VulkanDevice* device );
    ~EngineDataWindow() = default;

    auto render() -> void override;

  private:
    graphics::VulkanDevice* m_device;
    EngineDataSpec m_spec;
    double m_smoothedDelta;
    double m_timeUpdate;
};
} // namespace gui