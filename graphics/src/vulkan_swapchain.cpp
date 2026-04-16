#include "graphics/vulkan_swapchain.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <spdlog/spdlog.h>
#include "graphics/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include "precompiled/pch.hpp"

auto graphics::VulkanSwapchain::querySwapchainSupport() -> SwapchainSupportDetails
{
  SwapchainSupportDetails details;
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
    m_pDevice->getPhysicalDevice(), m_pDevice->getSurface(), &formatCount, nullptr );

  if ( formatCount != 0 )
  {
    details.formats.resize( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR(
      m_pDevice->getPhysicalDevice(), m_pDevice->getSurface(), &formatCount, details.formats.data() );
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
    m_pDevice->getPhysicalDevice(), m_pDevice->getSurface(), &presentModeCount, nullptr );

  if ( presentModeCount != 0 )
  {
    details.presentModes.resize( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      m_pDevice->getPhysicalDevice(), m_pDevice->getSurface(), &presentModeCount, details.presentModes.data() );
  }

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    m_pDevice->getPhysicalDevice(), m_pDevice->getSurface(), &details.capabilities );

  return details;
}

graphics::VulkanSwapchain::VulkanSwapchain( VulkanDevice* vulkanDevice, SDL_Window* wnd )
    : m_pDevice{ vulkanDevice }
    , m_window{ wnd }
{
  createSwapchain();
  createImageViews();
  createCommandPool();
  createCommandBuffers();
  createSyncObjects();
}

graphics::VulkanSwapchain::~VulkanSwapchain()
{
  destroy();
}

auto graphics::VulkanSwapchain::getCurrentCommandBuffer() -> VkCommandBuffer&
{
  return m_commandBuffers.at( m_currentFrame );
}

void graphics::VulkanSwapchain::onUpdate()
{
  m_currentFrame = ( m_currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
}

void graphics::VulkanSwapchain::recreateSwapchain()
{
  // this makes the app wait for as long as the window is minimized
  // and right as we resize the window or bring it to focus the swapchain
  // gets created
  while ( SDL_GetWindowFlags( m_window ) & SDL_WINDOW_MINIMIZED )
  {
    SDL_Event e;
    SDL_WaitEvent( &e );
  }

  vkDeviceWaitIdle( m_pDevice->getLogicalDevice() );

  destroy();
  createSwapchain();
  createImageViews();
  createCommandPool();
  createCommandBuffers();
  createSyncObjects();
}

void graphics::VulkanSwapchain::presentFrame()
{
  VkImageMemoryBarrier swapchainToPresent{};
  swapchainToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  swapchainToPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  swapchainToPresent.dstAccessMask = 0;
  swapchainToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  swapchainToPresent.subresourceRange.baseMipLevel = 0;
  swapchainToPresent.subresourceRange.levelCount = 1;
  swapchainToPresent.subresourceRange.baseArrayLayer = 0;
  swapchainToPresent.subresourceRange.layerCount = 1;

  swapchainToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  swapchainToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  swapchainToPresent.image = m_swapchainImages.at( m_imageIndex ).image;

  vkCmdPipelineBarrier( *m_currentCmdBuffer,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        0,
                        0,
                        nullptr,
                        0,
                        nullptr,
                        1,
                        &swapchainToPresent );

  // get the command buffer out of the recording state
  endCommandBuffer();

  // submit the command buffer to the queue
  submit();

  // present the result
  VkPresentInfoKHR presentInfo{
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &m_swapchainImages.at( m_imageIndex ).renderingFinished,
    .swapchainCount = 1,
    .pSwapchains = &m_swapchain,
    .pImageIndices = &m_imageIndex,
  };

  auto result = vkQueuePresentKHR( m_pDevice->getPresentQueue().handle, &presentInfo );
  if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
  {
    recreateSwapchain();
  }
  else if ( result != VK_SUCCESS )
  {
    throw std::runtime_error( "failed to present swap chain image!" );
  }

  m_currentFrame = ( m_currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
}

void graphics::VulkanSwapchain::beginCommandBuffer()
{
  m_currentCmdBuffer = &getCurrentCommandBuffer();
  VkCommandBufferBeginInfo begin{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  vkBeginCommandBuffer( *m_currentCmdBuffer, &begin );
}

bool graphics::VulkanSwapchain::beginRendering( const VkRenderingInfo& info )
{
  vkCmdBeginRendering( *m_currentCmdBuffer, &info );
  return true;
}

void graphics::VulkanSwapchain::endRendering()
{
  vkCmdEndRendering( *m_currentCmdBuffer );
}

auto graphics::VulkanSwapchain::getSwapchainImageFormat() -> VkFormat&
{
  return m_swapchainFormat;
}

void graphics::VulkanSwapchain::setupViewport( VkCommandBuffer& cmd )
{

  VkViewport viewport{ 0, 0, (float)m_swapchainExtent.width, (float)m_swapchainExtent.height, 0.0f, 1.0f };
  vkCmdSetViewport( cmd, 0, 1, &viewport );
}

void graphics::VulkanSwapchain::setupScissors( VkCommandBuffer& cmd )
{

  VkRect2D scissor{ { 0, 0 }, m_swapchainExtent };
  vkCmdSetScissor( cmd, 0, 1, &scissor );
}

void graphics::VulkanSwapchain::destroy()
{
  for ( auto& entry : m_swapchainImages )
  {
    vkDestroySemaphore( m_pDevice->getLogicalDevice(), entry.imageAvailable, nullptr );
    vkDestroySemaphore( m_pDevice->getLogicalDevice(), entry.renderingFinished, nullptr );
    vkDestroyFence( m_pDevice->getLogicalDevice(), entry.inFlight, nullptr );
    vkDestroyImageView( m_pDevice->getLogicalDevice(), entry.imageView, nullptr );
  }

  vkDestroySwapchainKHR( m_pDevice->getLogicalDevice(), m_swapchain, nullptr );
}

void graphics::VulkanSwapchain::submit()
{
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &m_swapchainImages.at( m_imageIndex ).renderingFinished;
  submitInfo.pWaitSemaphores = &m_swapchainImages.at( m_currentFrame ).imageAvailable;
  submitInfo.pCommandBuffers = &m_commandBuffers.at( m_currentFrame );
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.signalSemaphoreCount = 1;

  VK_CALL( vkQueueSubmit(
    m_pDevice->getGraphicsQueue().handle, 1, &submitInfo, m_swapchainImages.at( m_currentFrame ).inFlight ) );
}

void graphics::VulkanSwapchain::createSwapchain()
{
  SwapchainSupportDetails swapchainSupport = querySwapchainSupport();
  auto surfaceFormat = chooseSwapSurfaceFormat( swapchainSupport.formats );
  auto presentMode = chooseSwapPresentMode( swapchainSupport.presentModes );
  auto extent = chooseSwapExtent( swapchainSupport.capabilities );

  m_imageCount = swapchainSupport.capabilities.minImageCount + 1;

  if ( swapchainSupport.capabilities.maxImageCount > 0 && m_imageCount > swapchainSupport.capabilities.maxImageCount )
  {
    m_imageCount = swapchainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_pDevice->getSurface();

  createInfo.minImageCount = m_imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = { m_pDevice->getGraphicsQueue().familyIndex,
                                    m_pDevice->getPresentQueue().familyIndex };

  if ( queueFamilyIndices[0] != queueFamilyIndices[1] )
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  VK_CALL( vkCreateSwapchainKHR( m_pDevice->getLogicalDevice(), &createInfo, nullptr, &m_swapchain ) );
  VK_CALL( vkGetSwapchainImagesKHR( m_pDevice->getLogicalDevice(), m_swapchain, &m_imageCount, nullptr ) );
  std::vector<VkImage> images( m_imageCount );
  m_swapchainImages.resize( m_imageCount );
  VK_CALL( vkGetSwapchainImagesKHR( m_pDevice->getLogicalDevice(), m_swapchain, &m_imageCount, images.data() ) );
  for ( auto i = 0u; i < m_imageCount; i++ )
  {
    m_swapchainImages.at( i ).image = images.at( i );
  }

  m_swapchainFormat = surfaceFormat.format;
  m_swapchainExtent = extent;
}

void graphics::VulkanSwapchain::createImageViews()
{
  for ( size_t i = 0; i < m_imageCount; i++ )
  {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_swapchainImages[i].image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_swapchainFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VK_CALL(
      vkCreateImageView( m_pDevice->getLogicalDevice(), &createInfo, nullptr, &m_swapchainImages.at( i ).imageView ) );
  }
}

void graphics::VulkanSwapchain::createCommandPool()
{
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = m_pDevice->getGraphicsQueue().familyIndex;
  VK_CALL( vkCreateCommandPool( m_pDevice->getLogicalDevice(), &poolInfo, nullptr, &m_commandPool ) );
}

void graphics::VulkanSwapchain::createCommandBuffers()
{
  m_commandBuffers.resize( m_imageCount );
  for ( auto i = 0u; i < m_imageCount; i++ )
  {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VK_CALL( vkAllocateCommandBuffers( m_pDevice->getLogicalDevice(), &allocInfo, &m_commandBuffers.at( i ) ) );
  }
}

void graphics::VulkanSwapchain::createSyncObjects()
{
  for ( auto i = 0u; i < m_imageCount; i++ )
  {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CALL( vkCreateSemaphore(
      m_pDevice->getLogicalDevice(), &semaphoreInfo, nullptr, &m_swapchainImages.at( i ).imageAvailable ) );

    VK_CALL( vkCreateSemaphore(
      m_pDevice->getLogicalDevice(), &semaphoreInfo, nullptr, &m_swapchainImages.at( i ).renderingFinished ) );

    VK_CALL( vkCreateFence( m_pDevice->getLogicalDevice(), &fenceInfo, nullptr, &m_swapchainImages.at( i ).inFlight ) );
  }
}

auto graphics::VulkanSwapchain::chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities ) -> VkExtent2D
{
  if ( capabilities.currentExtent.width != UINT32_MAX )
  {
    return capabilities.currentExtent;
  }
  else
  {
    int width, height;
    SDL_GetWindowSize( m_window, &width, &height );

    VkExtent2D actualExtent = { static_cast<uint32_t>( width ), static_cast<uint32_t>( height ) };

    actualExtent.width =
      std::clamp( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
    actualExtent.height =
      std::clamp( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );

    return actualExtent;
  }
}

auto graphics::VulkanSwapchain::chooseSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes )
  -> VkPresentModeKHR
{
  for ( const auto& availablePresentMode : availablePresentModes )
  {
    if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
    {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

auto graphics::VulkanSwapchain::chooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableFormats )
  -> VkSurfaceFormatKHR
{
  for ( const auto& availableFormat : availableFormats )
  {
    if ( availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
         availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
    {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

auto graphics::VulkanSwapchain::getCommandPool() -> VkCommandPool&
{
  return m_commandPool;
}

auto graphics::VulkanSwapchain::getSwapchainExtent() -> VkExtent2D&
{
  return m_swapchainExtent;
}

auto graphics::VulkanSwapchain::getCurrentFrameIndex() const -> uint32_t
{
  return m_imageIndex;
}

bool graphics::VulkanSwapchain::aquireNextImage()
{
  auto result = vkAcquireNextImageKHR( m_pDevice->getLogicalDevice(),
                                       m_swapchain,
                                       UINT64_MAX,
                                       m_swapchainImages.at( m_currentFrame ).imageAvailable,
                                       VK_NULL_HANDLE,
                                       &m_imageIndex );

  if ( result == VK_ERROR_OUT_OF_DATE_KHR )
  {
    recreateSwapchain();
    return false;
  }
  else if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
  {
    throw std::runtime_error( "failed to acquire swapchain image!" );
  }
  return true;
}

void graphics::VulkanSwapchain::resetFences()
{
  vkResetFences( m_pDevice->getLogicalDevice(), 1, &m_swapchainImages.at( m_currentFrame ).inFlight );
}

void graphics::VulkanSwapchain::waitForFences()
{
  vkWaitForFences(
    m_pDevice->getLogicalDevice(), 1, &m_swapchainImages.at( m_currentFrame ).inFlight, VK_TRUE, UINT64_MAX );
}

auto graphics::VulkanSwapchain::getCurrentFrame() -> SwapchainImage&
{
  return m_swapchainImages.at( m_currentFrame );
}

void graphics::VulkanSwapchain::endCommandBuffer()
{
  vkEndCommandBuffer( *m_currentCmdBuffer );
}
