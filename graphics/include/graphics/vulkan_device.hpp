#pragma once
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

struct SDL_Window;

inline const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
inline const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                           VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct VulkanPlatform
{
  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
};

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() const
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct GPUQueue
{
  VkQueue handle{ VK_NULL_HANDLE };
  uint32_t familyIndex = UINT32_MAX;
};

namespace graphics
{
class VulkanDevice
{
public:
  explicit VulkanDevice( SDL_Window* wnd );
  ~VulkanDevice() = default;

  void init();
  void setupDebug();
  void createInstance();
  void createWindowSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void shutdown() const;

  auto getGraphicsQueue() -> GPUQueue&;
  auto getPresentQueue() -> GPUQueue&;
  auto getPhysicalDevice() -> VkPhysicalDevice&;
  auto getLogicalDevice() -> VkDevice&;
  auto getSurface() -> VkSurfaceKHR&;
  auto getInstance() -> VkInstance&;

private:
  auto findQueueFamilies( VkPhysicalDevice& device ) -> QueueFamilyIndices;
  bool isDeviceSuitable( VkPhysicalDevice& device );

  auto getRequiredExtensions() -> std::vector<const char*>;

private:
  SDL_Window* m_window;
  VulkanPlatform m_platform;
  VkPhysicalDeviceProperties m_physicalDeviceProps;
  VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProps;
  VkPhysicalDeviceFeatures m_physicalDeviceFeatures;

  VkDebugUtilsMessengerEXT m_debugMessenger{ VK_NULL_HANDLE };

  GPUQueue m_presentQueue;
  GPUQueue m_graphicsQueue;
};
} // namespace graphics
