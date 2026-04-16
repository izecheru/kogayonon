#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "precompiled/pch.hpp"

namespace graphics
{
class VulkanDevice;
class VulkanSwapchain;
} // namespace graphics

namespace core
{

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof( Vertex );
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof( Vertex, pos );

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof( Vertex, color );

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof( Vertex, texCoord );

    return attributeDescriptions;
  }
};

const std::vector<Vertex> vertices = { { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
                                       { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
                                       { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
                                       { { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },

                                       { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
                                       { { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
                                       { { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
                                       { { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } } };

const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };

class SceneRenderingSystem
{
public:
  explicit SceneRenderingSystem( graphics::VulkanDevice* pDevice, graphics::VulkanSwapchain* pSwapchain );
  ~SceneRenderingSystem() = default;

  void render( VkCommandBuffer& cmd );

private:
  void createGraphicsPipeline();
  void createDescriptorPool();
  void createDescriptorSetLayout();

private:
  // TODO(kogayonon) the pipeline should be stored elsewhere and then make the system use it
  VkPipelineLayout m_pipelineLayout;
  VkPipeline m_basicGeometryPipeline;
  VkDescriptorSetLayout m_descriptorSetLayout;
  VkDescriptorPool m_descriptorPool;

  VkBuffer m_stageBuffer;
  VkDeviceMemory m_stageBufferMemory;

  VkBuffer m_vertexBuffer;
  VkDeviceMemory m_vertexBufferMemory;

  graphics::VulkanDevice* m_pDevice;
  graphics::VulkanSwapchain* m_pSwapchain;
};
} // namespace core