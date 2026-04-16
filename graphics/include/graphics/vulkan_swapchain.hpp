#pragma once
#include <vulkan/vulkan.h>
#include "precompiled/pch.hpp"

#define MAX_FRAMES_IN_FLIGHT 3

namespace graphics
{
class VulkanDevice;
} // namespace graphics

struct QueueFamilyIndices;
struct SDL_Window;

struct SwapchainImage
{
  VkImage image{ VK_NULL_HANDLE };
  VkImageView imageView{ VK_NULL_HANDLE };

  VkSemaphore imageAvailable{};
  VkSemaphore renderingFinished{};
  VkFence inFlight{ VK_NULL_HANDLE };
};

struct SwapchainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

namespace graphics
{

class VulkanSwapchain
{
public:
  explicit VulkanSwapchain( VulkanDevice* vulkanDevice, SDL_Window* wnd );
  ~VulkanSwapchain();

  /**
   * @brief Get the command buffer at m_currentFrame index
   * @return A VkCommandBuffer to record commands to
   */
  auto getCurrentCommandBuffer() -> VkCommandBuffer&;
  void beginCommandBuffer();
  void endCommandBuffer();

  void onUpdate();

  /**
   * @brief Destroy and initialize the swapchain again
   */
  void recreateSwapchain();

  /**
   * @brief Present swapchain image to the surface
   */
  void presentFrame();

  bool beginRendering( const VkRenderingInfo& info );
  void endRendering();
  bool aquireNextImage();
  void resetFences();
  void waitForFences();

  auto getSwapchainImageFormat() -> VkFormat&;
  auto getCommandPool() -> VkCommandPool&;
  auto getSwapchainExtent() -> VkExtent2D&;
  auto getCurrentFrameIndex() const -> uint32_t;
  auto getCurrentFrame() -> SwapchainImage&;

  void setupViewport( VkCommandBuffer& cmd );
  void setupScissors( VkCommandBuffer& cmd );

private:
  void destroy();

  /**
   * @brief Submit command buffer to graphics queue
   */
  void submit();

  void createSwapchain();
  void createImageViews();
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();

  auto chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities ) -> VkExtent2D;
  auto chooseSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes ) -> VkPresentModeKHR;
  auto chooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableFormats ) -> VkSurfaceFormatKHR;
  auto querySwapchainSupport() -> SwapchainSupportDetails;

private:
  SDL_Window* m_window;
  bool m_destroyed{ false };
  VulkanDevice* m_pDevice;
  VkSwapchainKHR m_swapchain;
  std::vector<SwapchainImage> m_swapchainImages;

  VkFormat m_swapchainFormat;
  VkExtent2D m_swapchainExtent;

  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_imageCount;
  uint32_t m_currentFrame{ 0u };
  uint32_t m_imageIndex{ 0u };

  std::vector<VkCommandBuffer> m_commandBuffers;
  VkCommandPool m_commandPool;

  /**
   * @brief I use this just for convenience sake, it's not appealing to do auto& cmd = getCurrentCommandBuffer()
   * in each function I need
   */
  VkCommandBuffer* m_currentCmdBuffer;

  bool m_rendering{ true };
};
} // namespace graphics
