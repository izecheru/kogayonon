#include "graphics/vulkan_swapchain.hpp"
#include "graphics/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include "precompiled/pch.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

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

auto graphics::VulkanSwapchain::getCurrentCommandBuffer() -> VkCommandBuffer
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
  preparePresent();
  // get the command buffer out of the recording state
  endCommandBuffer();

  // submit the command buffer to the queue
  submit();

  // present the result
  VkPresentInfoKHR presentInfo{
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &m_renderingFinished.at( m_currentFrame ),
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

  onUpdate();
}

void graphics::VulkanSwapchain::beginCommandBuffer()
{
  m_currentCmdBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo begin{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  vkBeginCommandBuffer( m_currentCmdBuffer, &begin );
}

auto graphics::VulkanSwapchain::beginRendering( const VkRenderingInfo& info ) const -> void
{
  vkCmdBeginRendering( m_currentCmdBuffer, &info );
}

void graphics::VulkanSwapchain::endRendering() const
{
  vkCmdEndRendering( m_currentCmdBuffer );
}

auto graphics::VulkanSwapchain::getSwapchainImageFormat() -> VkFormat&
{
  return m_swapchainFormat;
}

void graphics::VulkanSwapchain::setupViewport( VkCommandBuffer cmd )
{
  VkViewport viewport{ 0, 0, (float)m_swapchainExtent.width, (float)m_swapchainExtent.height, 0.0f, 1.0f };
  vkCmdSetViewport( cmd, 0, 1, &viewport );
}

void graphics::VulkanSwapchain::setupViewport( VkCommandBuffer cmd, const VkExtent2D& extent )
{
  VkViewport viewport{ 0, 0, static_cast<float>( extent.width ), static_cast<float>( extent.height ), 0.0f, 1.0f };
  vkCmdSetViewport( cmd, 0, 1, &viewport );
}

void graphics::VulkanSwapchain::setupScissors( VkCommandBuffer cmd )
{
  VkRect2D scissor{ { 0, 0 }, m_swapchainExtent };
  vkCmdSetScissor( cmd, 0, 1, &scissor );
}

void graphics::VulkanSwapchain::setupScissors( VkCommandBuffer cmd, const VkExtent2D& extent )
{
  VkRect2D scissor{ { static_cast<int32_t>( 0 ), static_cast<int32_t>( 0 ) },
                    { static_cast<uint32_t>( extent.width ), static_cast<uint32_t>( extent.height ) } };
  vkCmdSetScissor( cmd, 0, 1, &scissor );
}

void graphics::VulkanSwapchain::destroy()
{
  m_pDevice->waitIdle();

  m_commandBuffers.clear();

  for ( auto& entry : m_swapchainImages )
  {
    m_pDevice->destroyImageView( entry.vkImageView );
  }

  m_pDevice->destroyCommandPool( m_commandPool );

  for ( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; ++i )
  {
    m_pDevice->destroySemaphore( m_imageAvailable.at( i ) );
    m_pDevice->destroySemaphore( m_renderingFinished.at( i ) );
    m_pDevice->destroyFence( m_inFlight.at( i ) );
  }

  m_pDevice->destroySwapchain( m_swapchain );
}

void graphics::VulkanSwapchain::submit()
{
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &m_renderingFinished.at( m_currentFrame );
  submitInfo.pWaitSemaphores = &m_imageAvailable.at( m_currentFrame );
  submitInfo.pCommandBuffers = &m_commandBuffers.at( m_currentFrame );
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.signalSemaphoreCount = 1;

  VK_CALL( vkQueueSubmit( m_pDevice->getGraphicsQueue().handle, 1, &submitInfo, m_inFlight.at( m_currentFrame ) ) );
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

  m_pDevice->createSwapchain( createInfo, &m_swapchain );
  m_pDevice->getSwapchainImagesKHR( m_swapchainImages, m_swapchain, m_imageCount );

  m_swapchainFormat = surfaceFormat.format;
  m_swapchainExtent = extent;
}

void graphics::VulkanSwapchain::createImageViews()
{
  for ( size_t i = 0; i < m_imageCount; i++ )
  {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_swapchainImages[i].vkImage;
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

    VK_CALL( vkCreateImageView(
      m_pDevice->getLogicalDevice(), &createInfo, nullptr, &m_swapchainImages.at( i ).vkImageView ) );
  }
}

void graphics::VulkanSwapchain::createCommandPool()
{
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = m_pDevice->getGraphicsQueue().familyIndex;
  m_pDevice->createCommandPool( poolInfo, m_commandPool );
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
  m_imageAvailable.resize( MAX_FRAMES_IN_FLIGHT );
  m_inFlight.resize( MAX_FRAMES_IN_FLIGHT );
  m_renderingFinished.resize( MAX_FRAMES_IN_FLIGHT );

  for ( auto i = 0u; i < m_imageCount; i++ )
  {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CALL( vkCreateSemaphore( m_pDevice->getLogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailable.at( i ) ) );

    VK_CALL(
      vkCreateSemaphore( m_pDevice->getLogicalDevice(), &semaphoreInfo, nullptr, &m_renderingFinished.at( i ) ) );

    VK_CALL( vkCreateFence( m_pDevice->getLogicalDevice(), &fenceInfo, nullptr, &m_inFlight.at( i ) ) );
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

auto graphics::VulkanSwapchain::getCommandPool() -> VkCommandPool
{
  return m_commandPool;
}

auto graphics::VulkanSwapchain::getSwapchainExtent() -> VkExtent2D&
{
  return m_swapchainExtent;
}

auto graphics::VulkanSwapchain::getAquiredImageIndex() const -> uint32_t
{
  return m_imageIndex;
}

bool graphics::VulkanSwapchain::aquireNextImage()
{
  auto result = vkAcquireNextImageKHR( m_pDevice->getLogicalDevice(),
                                       m_swapchain,
                                       UINT64_MAX,
                                       m_imageAvailable.at( m_currentFrame ),
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
  vkResetFences( m_pDevice->getLogicalDevice(), 1, &m_inFlight.at( m_currentFrame ) );
}

void graphics::VulkanSwapchain::waitForFences()
{
  vkWaitForFences( m_pDevice->getLogicalDevice(), 1, &m_inFlight.at( m_currentFrame ), VK_TRUE, UINT64_MAX );
}

auto graphics::VulkanSwapchain::getImageAtAquiredIndex() -> VulkanImage&
{
  return m_swapchainImages.at( m_imageIndex );
}

void graphics::VulkanSwapchain::endCommandBuffer() const
{
  vkEndCommandBuffer( m_currentCmdBuffer );
}

auto graphics::VulkanSwapchain::preparePresent() -> void
{
  VulkanImage& currentImage = getImageAtAquiredIndex();

  ImageTransitionData newTransition{};
  newTransition.srcAccess = currentImage.transition.newAccess;
  newTransition.newAccess = VK_ACCESS_2_NONE;
  newTransition.srcStage = currentImage.transition.newStage;
  newTransition.newStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
  newTransition.oldLayout = currentImage.transition.newLayout;
  newTransition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  currentImage.transition = newTransition;
  m_pDevice->transitionImageLayout( currentImage, m_currentCmdBuffer, currentImage.transition );
}

auto graphics::VulkanSwapchain::prepareAttachment() -> void
{
  VulkanImage& currentImage = getImageAtAquiredIndex();
  ImageTransitionData newTransition{};

  if ( currentImage.transition.newLayout == VK_IMAGE_LAYOUT_UNDEFINED )
  {
    newTransition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  }
  else if ( currentImage.transition.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR )
  {
    newTransition.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  }

  newTransition.srcAccess = VK_ACCESS_2_NONE;
  newTransition.newAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  newTransition.newStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  newTransition.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  currentImage.transition = newTransition;

  m_pDevice->transitionImageLayout( currentImage, m_currentCmdBuffer, currentImage.transition );
}

auto graphics::VulkanSwapchain::getCurrentFrameNumber() const -> uint32_t
{
  return m_currentFrame;
}
