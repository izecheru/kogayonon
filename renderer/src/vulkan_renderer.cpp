#include "renderer/vulkan_renderer.hpp"
#include "graphics/vulkan_context.hpp"

rendering::VulkanRenderer::VulkanRenderer(
  const std::initializer_list<std::pair<PipelineType, graphics::VulkanPipelineSpec>>& pipelineInitializer,
  graphics::VulkanContext* pCtx )
    : m_pVkContext{ pCtx }
{
  for ( auto& spec : pipelineInitializer )
  {
    // second is the spec and first is the type
    createPipeline( spec.second, spec.first );
  }
}

rendering::VulkanRenderer::~VulkanRenderer()
{
}

void rendering::VulkanRenderer::render()
{
  // now render here based on pipeline type
}

void rendering::VulkanRenderer::createPipeline( const graphics::VulkanPipelineSpec& spec,
                                                const PipelineType& pipelineType )
{
  m_pipelines.emplace( pipelineType, graphics::VulkanPipeline{ spec, m_pVkContext } );
}

auto rendering::VulkanRenderer::getViewport() -> VulkanViewport&
{
  return m_viewport;
}

void rendering::VulkanRenderer::createViewport()
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = m_pVkContext->swapchain->getSwapchainExtent().width;
  imageInfo.extent.height = m_pVkContext->swapchain->getSwapchainExtent().height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = m_pVkContext->swapchain->getSwapchainImageFormat();
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo imageAllocInfo{};
  imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  m_pVkContext->memoryAllocator->createImage( m_viewport.image, imageInfo, imageAllocInfo, m_viewport.allocation );

  m_viewport.imageView = createImageView(
    m_pVkContext, m_viewport.image, m_pVkContext->swapchain->getSwapchainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT );

  auto depthFormat = findDepthFormat( &m_pVkContext->device->getPhysicalDevice() );

  VkImageCreateInfo depthImageInfo{};
  depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
  depthImageInfo.extent.width = m_pVkContext->swapchain->getSwapchainExtent().width;
  depthImageInfo.extent.height = m_pVkContext->swapchain->getSwapchainExtent().height;
  depthImageInfo.extent.depth = 1;
  depthImageInfo.mipLevels = 1;
  depthImageInfo.arrayLayers = 1;
  depthImageInfo.format = depthFormat;
  depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo depthAllocInfo{};
  depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  m_pVkContext->memoryAllocator->createImage(
    m_viewport.depthImage, depthImageInfo, depthAllocInfo, m_viewport.depthAllocation );

  m_viewport.depthView = createImageView( m_pVkContext, m_viewport.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT );
}
