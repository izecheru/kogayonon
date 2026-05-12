#include "core/systems/scene_rendering_system.hpp"
#include "graphics/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "precompiled/pch.hpp"

core::SceneRenderingSystem::SceneRenderingSystem( graphics::VulkanDevice* pDevice,
                                                  graphics::VulkanSwapchain* pSwapchain )
    : m_pDevice{ pDevice }
    , m_pSwapchain{ pSwapchain }
{
  createDescriptorPool();
  createDescriptorSetLayout();
  createGraphicsPipeline();
}

void core::SceneRenderingSystem::render( VkCommandBuffer& cmd )
{
  // TODO(kogayonon) render entities from the current scene
  VkBuffer vertexBuffers[] = { m_vertexBuffer };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers( cmd, 0, 1, vertexBuffers, nullptr );
}

void core::SceneRenderingSystem::createGraphicsPipeline()
{
}

void core::SceneRenderingSystem::createDescriptorPool()
{
}

void core::SceneRenderingSystem::createDescriptorSetLayout()
{
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>( bindings.size() );
  layoutInfo.pBindings = bindings.data();

  if ( vkCreateDescriptorSetLayout( m_pDevice->getLogicalDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout ) !=
       VK_SUCCESS )
  {
    throw std::runtime_error( "failed to create descriptor set layout!" );
  }
}
