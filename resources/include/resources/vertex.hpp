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
  // because any vec3 turns to vec4 in the shader for memory alignment i chose to use the last value to write the
  // texture coordinates for the texture mapping
  glm::vec3 translation;
  float uvX;
  glm::vec3 normal;
  float uvY;

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
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{ 4 };

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VERTEX_VEC3;
    attributeDescriptions[0].offset = offsetof( Vertex, translation );

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VERTEX_FLOAT;
    attributeDescriptions[1].offset = offsetof( Vertex, uvX );

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VERTEX_VEC3;
    attributeDescriptions[2].offset = offsetof( Vertex, normal );

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VERTEX_VEC2;
    attributeDescriptions[3].offset = offsetof( Vertex, uvY );

    return attributeDescriptions;
  }
};
} // namespace resources