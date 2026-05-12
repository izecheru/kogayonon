#include "core/asset_manager/asset_manager.hpp"
#include "utilities/utils/utils.hpp"
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>
#define STB_IMAGE_IMPLEMENTATION
#include "graphics/utils.hpp"
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "resources/mesh.hpp"
#include "resources/texture.hpp"
#include <stb_image.h>

auto core::AssetManager::loadTexture( const std::string& textureName, const std::string& texturePath )
  -> resources::Texture*
{
  assert( std::filesystem::exists( texturePath ) && "texture file MUST EXIST" );

  if ( m_loadedTextures.contains( texturePath ) )
    return m_loadedTextures.at( texturePath ).get();

  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load( texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

  if ( !pixels )
  {
    spdlog::error( stbi_failure_reason() );
    throw std::runtime_error( "failed to load texture image!" );
  }

  auto texture = std::make_shared<resources::Texture>();
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  VkBufferCreateInfo stageBufferInfo{};
  stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  stageBufferInfo.size = imageSize;
  stageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  stageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

  VmaAllocationCreateInfo stageAllocInfo{};
  stageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
  // already mapped
  // stageAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  auto stageBuffer = m_pVkContext->memoryAllocator->createStagingBuffer( stageBufferInfo, stageAllocInfo );

  void* data;
  vmaMapMemory( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.allocation, &data );
  memcpy( data, pixels, (size_t)imageSize );
  vmaUnmapMemory( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.allocation );

  stbi_image_free( pixels );

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = texWidth;
  imageInfo.extent.height = texHeight;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 5;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo imageAllocInfo{};
  imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  m_pVkContext->memoryAllocator->createImage(
    texture->getImage(), imageInfo, imageAllocInfo, texture->getAllocation() );

  texture->getView() =
    createImageView( m_pVkContext, texture->getImage(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

  transitionImageLayout( m_pVkContext,
                         texture->getImage(),
                         VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

  copyBufferToImage( m_pVkContext,
                     stageBuffer.vkBuffer,
                     texture->getImage(),
                     static_cast<uint32_t>( texWidth ),
                     static_cast<uint32_t>( texHeight ) );

  transitionImageLayout( m_pVkContext,
                         texture->getImage(),
                         VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

  vmaDestroyBuffer( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.vkBuffer, stageBuffer.allocation );

  m_loadedTextures.emplace( texturePath, texture );

  return m_loadedTextures.at( texturePath ).get();
}

auto core::AssetManager::getTextureSampler() -> VkSampler&
{
  return m_textureSampler;
}

void core::AssetManager::initDescriptors()
{
  if ( !m_layoutInit )
  {
    createBindlessDescriptorSetLayout();
    allocateBindlessDescriptorSet();

    // materials
    createMaterialsBuffers();
    createMaterialsDescriptorSetLayout();
    allocateMaterialsDescriptorSet();
    createMaterialsDescriptorSet();

    m_layoutInit = true;
  }
}

void core::AssetManager::setContext( graphics::VulkanContext* ctx )
{
  m_pVkContext = ctx;
}

auto core::AssetManager::getTexture( const std::string& texturePath ) -> resources::Texture*
{
  return m_loadedTextures.at( texturePath ).get();
}

void core::AssetManager::initSampler()
{
  createSampler(
    m_pVkContext->device->getLogicalDevice(), m_pVkContext->device->getPhysicalDevice(), m_textureSampler );
}

auto core::AssetManager::loadMesh( const std::string& meshName, const std::string& meshPath ) -> resources::Mesh*
{
  if ( m_loadedMeshes.contains( meshPath ) )
    return m_loadedMeshes.at( meshPath ).get();

  assert( std::filesystem::exists( meshPath ) && "mesh file does not exist" );
  auto mesh = std::make_shared<resources::Mesh>();

  mesh->getPath() = meshPath;
  std::filesystem::path p{ meshPath };

  std::vector<aiMaterial*> materials{};
  // m_cgltfLoader.loadMesh( meshPath, mesh.get(), meshes );
  //  for now we use assimp for the broader spectrum of supported model file formats

  // we store the texture type and path for it
  m_assimpLoader.loadMesh( meshPath, mesh.get(), materials );
  auto& submeshes = mesh->getSubmeshes();

  for ( auto i = 0u; i < materials.size(); i++ )
  {

    resources::Material mat{};

    if ( materials.at( i )->GetTextureCount( aiTextureType_SPECULAR ) > 0 )
    {
      aiString str;
      materials.at( i )->GetTexture( aiTextureType_SPECULAR, 0, &str );

      auto path = std::filesystem::absolute( "." ) / ( "engine_resources\\" + std::string( str.C_Str() ) );

      auto key = std::filesystem::weakly_canonical( path ).string();

      if ( !m_loadedTextures.contains( key ) )
      {
        mat.specularTextureIndex = m_bindlessTexturesIndex;
        auto texture = loadTexture( path.stem().string(), key );
        texture->setIndex( m_bindlessTexturesIndex );
        updateBindlessTextures( texture );
        ++m_bindlessTexturesIndex;
      }
      else
      {
        mat.specularTextureIndex = getTexture( key )->getIndex();
      }
    }

    if ( materials.at( i )->GetTextureCount( aiTextureType_NORMALS ) > 0 )
    {
      aiString str;
      materials.at( i )->GetTexture( aiTextureType_NORMALS, 0, &str );

      auto path = std::filesystem::absolute( "." ) / ( "engine_resources\\" + std::string( str.C_Str() ) );

      auto key = std::filesystem::weakly_canonical( path ).string();

      if ( !m_loadedTextures.contains( key ) )
      {
        mat.normalTextureIndex = m_bindlessTexturesIndex;
        auto texture = loadTexture( path.stem().string(), key );
        texture->setIndex( m_bindlessTexturesIndex );
        updateBindlessTextures( texture );
        ++m_bindlessTexturesIndex;
      }
      else
      {
        mat.normalTextureIndex = getTexture( key )->getIndex();
      }
    }

    if ( materials.at( i )->GetTextureCount( aiTextureType_DIFFUSE ) > 0 )
    {
      aiString str;
      materials.at( i )->GetTexture( aiTextureType_DIFFUSE, 0, &str );

      auto path = std::filesystem::absolute( "." ) / ( "engine_resources\\" + std::string( str.C_Str() ) );

      auto key = std::filesystem::weakly_canonical( path ).string();

      if ( !m_loadedTextures.contains( key ) )
      {
        mat.diffuseTextureIndex = m_bindlessTexturesIndex;
        auto texture = loadTexture( path.stem().string(), key );
        texture->setIndex( m_bindlessTexturesIndex );
        updateBindlessTextures( texture );
        ++m_bindlessTexturesIndex;
      }
      else
      {
        mat.diffuseTextureIndex = getTexture( key )->getIndex();
      }
    }
    // now check if the material built is in the vector
    bool found = false;

    for ( auto& existing : m_materials )
    {
      if ( existing.diffuseTextureIndex == mat.diffuseTextureIndex &&
           existing.normalTextureIndex == mat.normalTextureIndex &&
           existing.specularTextureIndex == mat.specularTextureIndex )
      {
        submeshes[i].submeshMaterial = existing;
        found = true;
        break;
      }
    }

    if ( !found )
    {
      m_materials.push_back( mat );
      submeshes[i].submeshMaterial = mat;
      updateMaterialsBuffer();
    }
  }

  createVertexBuffer( mesh.get() );
  createIndexBuffer( mesh.get() );

  m_loadedMeshes.try_emplace( meshPath, mesh );

  return m_loadedMeshes.at( meshPath ).get();
}

auto core::AssetManager::getMesh( const std::string& path ) -> std::optional<resources::Mesh*>
{
  if ( m_loadedMeshes.contains( path ) )
    return std::optional<resources::Mesh*>( m_loadedMeshes.at( path ).get() );

  auto p = std::filesystem::path{ path };
  auto opt = std::optional<resources::Mesh*>{ loadMesh( p.stem().string(), p.string() ) };

  return opt;
}

void core::AssetManager::createIndexBuffer( resources::Mesh* pMesh )
{
  auto& indices = pMesh->getIndices();
  VkDeviceSize bufferSize = sizeof( uint32_t ) * indices.size();

  VkBufferCreateInfo stageBufferInfo{};
  stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  stageBufferInfo.size = bufferSize;
  stageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  stageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo stageAllocInfo{};
  stageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

  auto stageBuffer = m_pVkContext->memoryAllocator->createStagingBuffer( stageBufferInfo, stageAllocInfo );

  void* data;
  vmaMapMemory( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.allocation, &data );
  memcpy( data, indices.data(), (size_t)bufferSize );
  vmaUnmapMemory( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.allocation );

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  m_pVkContext->memoryAllocator->createBuffer( pMesh->getIndicesBufferObject(), bufferInfo, vmaAllocInfo, false );

  copyBuffer( m_pVkContext->swapchain->getCommandPool(),
              m_pVkContext->device->getLogicalDevice(),
              m_pVkContext->device->getGraphicsQueue().handle,
              stageBuffer.vkBuffer,
              pMesh->getIndicesBufferObject().vkBuffer,
              bufferSize );

  vmaDestroyBuffer( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.vkBuffer, stageBuffer.allocation );
}

void core::AssetManager::createVertexBuffer( resources::Mesh* pMesh )
{
  auto& vertices = pMesh->getVertices();
  VkDeviceSize bufferSize = sizeof( resources::Vertex ) * vertices.size();

  VkBufferCreateInfo stageBufferInfo{};
  stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  stageBufferInfo.size = bufferSize;
  stageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  stageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

  VmaAllocationCreateInfo stageAllocInfo{};
  stageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

  auto stageBuffer = m_pVkContext->memoryAllocator->createStagingBuffer( stageBufferInfo, stageAllocInfo );

  void* data;
  vmaMapMemory( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.allocation, &data );
  memcpy( data, vertices.data(), (size_t)bufferSize );
  vmaUnmapMemory( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.allocation );

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  m_pVkContext->memoryAllocator->createBuffer( pMesh->getVertexBufferObject(), bufferInfo, vmaAllocInfo, false );

  copyBuffer( m_pVkContext->swapchain->getCommandPool(),
              m_pVkContext->device->getLogicalDevice(),
              m_pVkContext->device->getGraphicsQueue().handle,
              stageBuffer.vkBuffer,
              pMesh->getVertexBufferObject().vkBuffer,
              bufferSize );

  vmaDestroyBuffer( m_pVkContext->memoryAllocator->getAllocator(), stageBuffer.vkBuffer, stageBuffer.allocation );
}

void core::AssetManager::createBindlessDescriptorSetLayout()
{
  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 0;
  // max textures number
  samplerLayoutBinding.descriptorCount = 1000;

  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                   VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                   VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

  VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
  bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
  bindingFlags.bindingCount = 1;
  bindingFlags.pBindingFlags = &flags;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &samplerLayoutBinding;
  layoutInfo.pNext = nullptr;
  layoutInfo.flags = 0;

  VK_CALL( vkCreateDescriptorSetLayout(
    m_pVkContext->device->getLogicalDevice(), &layoutInfo, nullptr, &m_bindlessTexturesDescriptorSetLayout ) );
}

void core::AssetManager::allocateBindlessDescriptorSet()
{
  uint32_t descriptorCount{ 1000 };
  std::vector<VkDescriptorSetLayout> layouts{ m_bindlessTexturesDescriptorSetLayout };

  VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
  variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
  variableCountInfo.descriptorSetCount = 1;
  variableCountInfo.pDescriptorCounts = &descriptorCount;

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

  // this is the global descriptor pool
  allocInfo.descriptorPool = *m_pDescriptorPool;
  allocInfo.descriptorSetCount = std::size( layouts );
  allocInfo.pSetLayouts = layouts.data();
  allocInfo.pNext = &variableCountInfo;

  VK_CALL(
    vkAllocateDescriptorSets( m_pVkContext->device->getLogicalDevice(), &allocInfo, &m_bindlessTextureDescriptorSet ) );
}

void core::AssetManager::setDescriptorPool( VkDescriptorPool* pool )
{
  m_pDescriptorPool = pool;
}

void core::AssetManager::updateBindlessTextures( resources::Texture* pTexture )
{
  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = pTexture->getView();
  imageInfo.sampler = m_textureSampler;

  auto bindlessDescriptor = VkWriteDescriptorSet{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                  .dstSet = m_bindlessTextureDescriptorSet,
                                                  .dstBinding = 0,
                                                  .dstArrayElement = m_bindlessTexturesIndex,
                                                  .descriptorCount = 1,
                                                  .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                  .pImageInfo = &imageInfo };

  VkWriteDescriptorSet descriptorWrites{ bindlessDescriptor };
  vkUpdateDescriptorSets( m_pVkContext->device->getLogicalDevice(), 1, &descriptorWrites, 0, nullptr );
}

void core::AssetManager::createMaterialsDescriptorSet()
{
  for ( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++ )
  {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_materialsBuffer.buffers.at( i ).vkBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof( resources::Material );

    auto shaderStorageBufferDescriptor = VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = m_materialsDescriptorSets[i],
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &bufferInfo,
    };

    VkWriteDescriptorSet descriptorWrites = shaderStorageBufferDescriptor;
    vkUpdateDescriptorSets( m_pVkContext->device->getLogicalDevice(), 1, &descriptorWrites, 0, nullptr );
  }
}

void core::AssetManager::createMaterialsDescriptorSetLayout()
{
  VkDescriptorSetLayoutBinding materialsBufferBinding{};
  materialsBufferBinding.binding = 0;
  materialsBufferBinding.descriptorCount = 1;

  // this is a storage buffer(ssbo)
  materialsBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  materialsBufferBinding.pImmutableSamplers = nullptr;
  materialsBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorBindingFlags flags = 0;

  VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
  bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
  bindingFlags.bindingCount = 1;
  bindingFlags.pBindingFlags = &flags;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &materialsBufferBinding;
  layoutInfo.pNext = &bindingFlags;
  layoutInfo.flags = 0;

  VK_CALL( vkCreateDescriptorSetLayout(
    m_pVkContext->device->getLogicalDevice(), &layoutInfo, nullptr, &m_materialsDescriptorSetLayout ) );
}

void core::AssetManager::allocateMaterialsDescriptorSet()
{
  std::vector<VkDescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT, m_materialsDescriptorSetLayout );

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = *m_pDescriptorPool;
  allocInfo.descriptorSetCount = std::size( layouts );
  allocInfo.pSetLayouts = layouts.data();
  allocInfo.pNext = nullptr;

  m_materialsDescriptorSets.resize( MAX_FRAMES_IN_FLIGHT );
  VK_CALL( vkAllocateDescriptorSets(
    m_pVkContext->device->getLogicalDevice(), &allocInfo, m_materialsDescriptorSets.data() ) );
}

void core::AssetManager::createMaterialsBuffers()
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof( resources::Material );
  bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  m_pVkContext->memoryAllocator->createBuffers( m_materialsBuffer, bufferInfo, vmaAllocInfo, true );
}

void core::AssetManager::updateMaterialsBuffer()
{
  vmaCopyMemoryToAllocation(
    m_pVkContext->memoryAllocator->getAllocator(),
    m_materials.data(),
    m_materialsBuffer.buffers.at( m_pVkContext->swapchain->getCurrentFrameIndex() ).allocation,
    0,
    sizeof( resources::Material ) * m_materials.size() );
}

auto core::AssetManager::getMaterialsDescriptorLayout() -> VkDescriptorSetLayout&
{
  return m_materialsDescriptorSetLayout;
}

auto core::AssetManager::getMaterialsDescriptorSet() -> VkDescriptorSet&
{
  return m_materialsDescriptorSets[m_pVkContext->swapchain->getCurrentFrameIndex()];
}

auto core::AssetManager::getBindlessDescriptorLayout() -> VkDescriptorSetLayout&
{
  return m_bindlessTexturesDescriptorSetLayout;
}

auto core::AssetManager::getBindlessDescriptorSet() -> VkDescriptorSet&
{
  return m_bindlessTextureDescriptorSet;
}
