#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace resources
{

// macros for vertex shader input types
enum class VertexDataType
{
  Float = VK_FORMAT_R32_SFLOAT,
  Vec2 = VK_FORMAT_R32G32_SFLOAT,
  Vec3 = VK_FORMAT_R32G32B32_SFLOAT,
  Vec4 = VK_FORMAT_R32G32B32A32_SFLOAT,
  Ivec2 = VK_FORMAT_R32G32_SINT,
  Uvec4 = VK_FORMAT_R32G32B32A32_UINT,
  Double = VK_FORMAT_R64_SFLOAT,
};

struct Vertex
{
  glm::vec3 translation;
  glm::vec3 normal;
  glm::vec2 uv;

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

    attributeDescriptions.at( 0 ) = { .location = 0,
                                      .binding = 0,
                                      .format = static_cast<VkFormat>( VertexDataType::Vec3 ),
                                      .offset = offsetof( Vertex, translation ) };

    attributeDescriptions.at( 1 ) = { .location = 1,
                                      .binding = 0,
                                      .format = static_cast<VkFormat>( VertexDataType::Vec3 ),
                                      .offset = offsetof( Vertex, normal ) };

    attributeDescriptions.at( 2 ) = { .location = 2,
                                      .binding = 0,
                                      .format = static_cast<VkFormat>( VertexDataType::Vec2 ),
                                      .offset = offsetof( Vertex, uv ) };

    return attributeDescriptions;
  }
};
} // namespace resources