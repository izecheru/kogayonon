#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace resources
{

// macros for vertex shader input types
#define VERTEX_FLOAT VK_FORMAT_R32_SFLOAT
#define VERTEX_VEC2 VK_FORMAT_R32G32_SFLOAT
#define VERTEX_VEC3 VK_FORMAT_R32G32B32_SFLOAT
#define VERTEX_VEC4 VK_FORMAT_R32G32B32A32_SFLOAT
#define VERTEX_IVEC2 VK_FORMAT_R32G32_SINT
#define VERTEX_UVEC4 VK_FORMAT_R32G32B32A32_UINT
#define VERTEX_DOUBLE VK_FORMAT_R64_SFLOAT

struct Vertex
{
  alignas( 16 ) glm::vec3 translation;
  alignas( 16 ) glm::vec3 normal;
  alignas( 16 ) glm::vec2 uv;

  static VkVertexInputBindingDescription getBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof( Vertex );
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
  {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions( 3 );

    attributeDescriptions[0] = {
      .location = 0, .binding = 0, .format = VERTEX_VEC3, .offset = offsetof( Vertex, translation ) };

    attributeDescriptions[1] = {
      .location = 1, .binding = 0, .format = VERTEX_VEC3, .offset = offsetof( Vertex, normal ) };

    attributeDescriptions[2] = { .location = 2, .binding = 0, .format = VERTEX_VEC2, .offset = offsetof( Vertex, uv ) };

    return attributeDescriptions;
  }
};
} // namespace resources