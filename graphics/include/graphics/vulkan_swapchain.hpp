#pragma once
#include "graphics/vulkan_defines.hpp"
#include "graphics/vulkan_image.hpp"
#include "precompiled/pch.hpp"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace graphics
{
class VulkanDevice;
} // namespace graphics

struct QueueFamilyIndices;
struct SDL_Window;

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
  auto getCurrentCommandBuffer() -> VkCommandBuffer;
  auto beginCommandBuffer() -> void;
  auto endCommandBuffer() const -> void;

  auto onUpdate() -> void;

  auto prepareAttachment() -> void;
  auto preparePresent() -> void;

  /**
   * @brief Destroy and initialize the swapchain again
   */
  auto recreateSwapchain() -> void;

  /**
   * @brief Present swapchain image to the surface
   */
  void presentFrame();

  /**
   * @brief Calls vkCmdBeginRendering on m_currentCmdBuffer
   * @param info Rendering information
   * @return
   */
  auto beginRendering( const VkRenderingInfo& info ) const -> void;

  /**
   * @brief Calls vkCmdEndRendering on m_currentCmdBuffer
   */
  void endRendering() const;

  bool aquireNextImage();
  auto resetFences() -> void;
  auto waitForFences() -> void;

  auto getSwapchainImageFormat() -> VkFormat&;
  auto getCommandPool() -> VkCommandPool;
  auto getSwapchainExtent() -> VkExtent2D&;
  auto getCurrentFrameNumber() const -> uint32_t;
  auto getAquiredImageIndex() const -> uint32_t;
  auto getImageAtAquiredIndex() -> VulkanImage&;

  auto setupViewport( VkCommandBuffer cmd ) -> void;
  auto setupScissors( VkCommandBuffer cmd ) -> void;

  auto setupViewport( VkCommandBuffer cmd, const VkExtent2D& extent ) -> void;
  auto setupScissors( VkCommandBuffer cmd, const VkExtent2D& extent ) -> void;

private:
  void destroy();

  /**
   * @brief Submit command buffer to graphics queue
   */
  void submit();

  auto createSwapchain() -> void;
  auto createImageViews() -> void;
  auto createCommandPool() -> void;
  auto createCommandBuffers() -> void;
  auto createSyncObjects() -> void;

  auto chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities ) -> VkExtent2D;
  auto chooseSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes ) -> VkPresentModeKHR;
  auto chooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableFormats ) -> VkSurfaceFormatKHR;
  auto querySwapchainSupport() -> SwapchainSupportDetails;

private:
  SDL_Window* m_window;
  bool m_destroyed{ false };
  VulkanDevice* m_pDevice;
  VkSwapchainKHR m_swapchain;
  std::vector<VulkanImage> m_swapchainImages;

  VkFormat m_swapchainFormat;
  VkExtent2D m_swapchainExtent;

  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_imageCount;
  uint32_t m_currentFrame{ 0u };
  uint32_t m_imageIndex{ 0u };

  std::vector<VkCommandBuffer> m_commandBuffers;
  VkCommandPool m_commandPool;

  VkCommandBuffer m_currentCmdBuffer;

  std::vector<VkSemaphore> m_imageAvailable;
  std::vector<VkSemaphore> m_renderingFinished;
  std::vector<VkFence> m_inFlight;
};
} // namespace graphics
