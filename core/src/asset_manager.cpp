#include <vulkan/vulkan.h>
#include "core/ecs/main_registry.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "resources/font.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "utilities/tracy_utils/tracy_utils.hpp"
#include "utilities/utils/utils.hpp"

core::AssetManager::AssetManager( graphics::VulkanContext* vkCtx )
    : m_bindlessTexturesIndex{ 0u }
    , m_samplerIndex{ 0u }
    , m_vkCtx{ vkCtx }
{
  initDescriptors();
  initSampler();
}

core::AssetManager::~AssetManager()
{
  m_vkCtx->device->waitIdle();

  m_vkCtx->device->destroyBuffer( m_materialsBuffer );
  m_vkCtx->device->destroySampler( m_textureSampler );
  m_vkCtx->device->destroyDescriptorSetLayout( m_bindlessTexturesDescriptor.layout );
  m_vkCtx->device->destroyDescriptorSetLayout( m_materialsDescriptor.layout );

  for ( auto& [path, texture] : m_loadedTextures )
  {
    m_vkCtx->device->destroyImageView( texture->getView() );
    m_vkCtx->device->destroyImage( texture->getImage(), texture->getAllocation() );
  }

  for ( auto& [path, mesh] : m_loadedMeshes )
  {
    m_vkCtx->device->destroyBuffer( mesh->getIndicesBufferObject() );
    m_vkCtx->device->destroyBuffer( mesh->getVertexBufferObject() );
  }
}

auto core::AssetManager::loadTextures( const std::vector<std::tuple<std::string, std::string>>& textures ) -> void
{
  ZoneScopedN( "AssetManager::loadTextures" );

  struct ImageData
  {
    int w;
    int h;
    int c;
    stbi_uc* data;
    VkDeviceSize size;
    resources::Texture* pTexture;
  };

  std::vector<ImageData> imageData{};
  std::vector<VkImageMemoryBarrier2> beforeBarriers{};
  std::vector<VkImageMemoryBarrier2> afterBarriers{};

  VkCommandPool commandPool = m_vkCtx->device->getCommandPool();
  VkCommandBuffer commandBuffer = m_vkCtx->device->beginSingleTimeCommands( commandPool );

  imageData.reserve( textures.size() );

  VkDeviceSize requiredStageBufferSize{ 0 };

  {
    ZoneScopedN( "AssetManager::loadTextures::stbi_load+VkImage+VkImageView" );
    for ( const auto& [path, name] : textures )
    {
      imageData.push_back( ImageData{} );

      ImageData& img = imageData.back();

      {
        ZoneScopedN( "stbi_load" );
        img.data = stbi_load( path.c_str(), &img.w, &img.h, &img.c, STBI_rgb_alpha );
      }

      img.size = img.w * img.h * 4;
      requiredStageBufferSize += img.size;

      if ( !img.data )
      {
        throw std::runtime_error( "failed to load texture image!" );
      }

      VkImageCreateInfo imageInfo{};
      imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageInfo.imageType = VK_IMAGE_TYPE_2D;
      imageInfo.extent.width = img.w;
      imageInfo.extent.height = img.h;
      imageInfo.extent.depth = 1;
      imageInfo.mipLevels = 1;
      imageInfo.arrayLayers = 1;
      imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
      imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
      imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      VmaAllocationCreateInfo imageAllocInfo{};
      imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;

      std::unique_ptr<resources::Texture> texture = std::make_unique<resources::Texture>();
      img.pTexture = texture.get();

      m_vkCtx->device->createImage( texture->getImage(), imageInfo, imageAllocInfo, texture->getAllocation(), name );

      m_vkCtx->device->createImageView(
        texture->getView(), texture->getImage(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

      beforeBarriers.emplace_back( VkImageMemoryBarrier2{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                                          .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                                                          .srcAccessMask = VK_ACCESS_2_NONE,
                                                          .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                                                          .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                          .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                          .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                          .image = texture->getImage(),
                                                          .subresourceRange = {
                                                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                            .baseMipLevel = 0,
                                                            .levelCount = 1,
                                                            .baseArrayLayer = 0,
                                                            .layerCount = 1,
                                                          } } );

      VkImageAspectFlags aspect{ VK_IMAGE_ASPECT_COLOR_BIT };
      if ( beforeBarriers.back().oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL )
      {
        aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
      }

      afterBarriers.emplace_back( VkImageMemoryBarrier2{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                                         .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                                                         .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                         .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                                         .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                                                         .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                         .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                         .image = texture->getImage(),
                                                         .subresourceRange = {
                                                           .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                           .baseMipLevel = 0,
                                                           .levelCount = 1,
                                                           .baseArrayLayer = 0,
                                                           .layerCount = 1,
                                                         } } );

      texture->setPath( path );

      m_loadedTextures.emplace( path, std::move( texture ) );
    }
  }

  // set all the undefined barriers here
  VkDependencyInfo before{};
  before.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  before.imageMemoryBarrierCount = beforeBarriers.size();
  before.pImageMemoryBarriers = beforeBarriers.data();

  // all the barriers at once
  vkCmdPipelineBarrier2( commandBuffer, &before );

  VkBufferCreateInfo stageBufferInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                      .size = requiredStageBufferSize,
                                      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      .sharingMode = VK_SHARING_MODE_EXCLUSIVE };

  VmaAllocationCreateInfo stageAllocInfo{ .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                   VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                          .usage = VMA_MEMORY_USAGE_AUTO };

  // since i want to use a single staging buffer we will use offsets and copy the data at those offsets
  graphics::VulkanBuffer stageBuffer = m_vkCtx->device->createStagingBuffer( stageBufferInfo, stageAllocInfo );

  VkDeviceSize currentOffset{ 0 };

  for ( auto& img : imageData )
  {
    m_vkCtx->device->copyMemoryToAllocation( img.data, stageBuffer.vmaAllocation, currentOffset, img.size );

    VkBufferImageCopy region{};
    region.bufferOffset = currentOffset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { static_cast<uint32_t>( img.w ), static_cast<uint32_t>( img.h ), 1 };

    currentOffset += img.size;

    vkCmdCopyBufferToImage(
      commandBuffer, stageBuffer.vkBuffer, img.pTexture->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
  }

  VkDependencyInfo after{};
  after.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  after.imageMemoryBarrierCount = afterBarriers.size();
  after.pImageMemoryBarriers = afterBarriers.data();

  vkCmdPipelineBarrier2( commandBuffer, &after );

  VkQueue graphicsQueue = m_vkCtx->device->getGraphicsQueue().handle;

  // this also waits on the queue since the staging buffer MUST live as long
  // as the commands registered in that command buffer do
  m_vkCtx->device->endSingleTimeCommands( commandBuffer, graphicsQueue, commandPool );

  m_vkCtx->device->destroyBuffer( stageBuffer );

  for ( ImageData& img : imageData )
  {
    updateBindlessTextures( img.pTexture );
    stbi_image_free( img.data );
  }
}

auto core::AssetManager::loadTexture( const std::string& textureName, const std::string& texturePath )
  -> resources::Texture*
{
  KASSERT( std::filesystem::exists( texturePath ) && "texture file MUST EXIST" );
  ZoneScopedN( "AssetManager::loadTexture" );

  if ( m_loadedTextures.contains( texturePath ) )
  {
    return m_loadedTextures[texturePath].get();
  }

  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load( texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

  if ( !pixels )
  {
    throw std::runtime_error( "failed to load texture image!" );
  }

  std::unique_ptr<resources::Texture> texture = std::make_unique<resources::Texture>();
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  VkBufferCreateInfo stageBufferInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                      .size = imageSize,
                                      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      .sharingMode = VK_SHARING_MODE_EXCLUSIVE };

  VmaAllocationCreateInfo stageAllocInfo{ .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                   VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                          .usage = VMA_MEMORY_USAGE_AUTO };

  graphics::VulkanBuffer stageBuffer = m_vkCtx->device->createStagingBuffer( stageBufferInfo, stageAllocInfo );

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = texWidth;
  imageInfo.extent.height = texHeight;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo imageAllocInfo{};
  imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;

  m_vkCtx->device->createImage( texture->getImage(), imageInfo, imageAllocInfo, texture->getAllocation(), textureName );

  m_vkCtx->device->createImageView(
    texture->getView(), texture->getImage(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

  // transfer barrier
  VkImageMemoryBarrier2 transferLayoutBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                               .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                                               .srcAccessMask = VK_ACCESS_2_NONE,
                                               .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                                               .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                               .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                               .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                               .image = texture->getImage(),
                                               .subresourceRange = {
                                                 .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                 .baseMipLevel = 0,
                                                 .levelCount = 1,
                                                 .baseArrayLayer = 0,
                                                 .layerCount = 1,
                                               } };

  m_vkCtx->device->transitionImageLayout( texture->getImage(), transferLayoutBarrier );

  m_vkCtx->device->copyMemoryToAllocation( pixels, stageBuffer.vmaAllocation, 0, imageSize );

  stbi_image_free( pixels );

  // copy and also transition layout
  VkImageMemoryBarrier2 copyImageBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                          .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                                          .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                          .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                          .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                                          .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          .image = texture->getImage(),
                                          .subresourceRange = {
                                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                            .baseMipLevel = 0,
                                            .levelCount = 1,
                                            .baseArrayLayer = 0,
                                            .layerCount = 1,
                                          } };

  m_vkCtx->device->copyBufferToImage( stageBuffer.vkBuffer,
                                      texture->getImage(),
                                      static_cast<uint32_t>( texWidth ),
                                      static_cast<uint32_t>( texHeight ),
                                      copyImageBarrier );

  m_vkCtx->device->destroyBuffer( stageBuffer );

  texture->setPath( texturePath );

  m_loadedTextures.emplace( texturePath, std::move( texture ) );

  return m_loadedTextures[texturePath].get();
}

auto core::AssetManager::getTextureSampler() -> VkSampler&
{
  return m_textureSampler;
}

auto core::AssetManager::initDescriptors() -> void
{
  if ( !m_layoutInit )
  {
    createBindlessDescriptorSetLayout();
    allocateBindlessDescriptorSet();

    // materials
    createMaterialsBuffers( 1000 * sizeof( resources::Material ) );
    createMaterialsDescriptorSetLayout();
    allocateMaterialsDescriptorSet();

    m_layoutInit = true;
  }
}

auto core::AssetManager::getTexture( const std::string& texturePath ) -> resources::Texture*
{
  std::filesystem::path p{ texturePath };

  if ( m_loadedTextures.contains( texturePath ) )
    return m_loadedTextures[texturePath].get();

  return nullptr;
}

auto core::AssetManager::initSampler() -> void
{
  m_vkCtx->device->createSampler( m_textureSampler, "assetManager_Sampler" );
  ++m_samplerIndex;
}

auto core::AssetManager::loadMesh( const std::string& meshName, const std::string& meshPath ) -> resources::Mesh*
{
  ZoneScopedN( "AssetManager::loadMesh" );
  assert( std::filesystem::exists( meshPath ) && "mesh file does not exist" );

  if ( m_loadedMeshes.contains( meshPath ) )
    return m_loadedMeshes[meshPath].get();

  if ( m_materials.empty() )
  {
    resources::Material defaultMaterial{};

    std::filesystem::path path = std::filesystem::current_path() / "engine_resources" / "textures" / "default.png";

    if ( std::filesystem::exists( path ) && !m_loadedTextures.contains( meshPath ) )
    {
      resources::Texture* texture = loadTexture( "default", path.string() );
      texture->setIndex( m_bindlessTexturesIndex );
      defaultMaterial.diffuseTextureIndex = m_bindlessTexturesIndex;
      updateBindlessTextures( texture );
    }

    m_materials.push_back( defaultMaterial );
    updateMaterialsBuffer();
  }

  m_loadedMeshes.emplace( meshPath, std::make_unique<resources::Mesh>() );

  resources::Mesh* mesh = m_loadedMeshes[meshPath].get();

  mesh->setPath( meshPath );

  utilities::TaskManager* taskManager = core::MainRegistry::getInstance().getTaskManager();

  utilities::CallbackTask* callbackPtr =
    taskManager->addTask( [this, meshPath, meshPtr = m_loadedMeshes[meshPath].get()]() -> void {
      ZoneScopedN( "Threaded loadMesh" );
      auto tinyLoader = std::make_unique<TinyGltfLoader>( meshPath );
      {
        std::lock_guard lock( m_mutex );
        tinyLoader->processVertexData( meshPtr );
        auto parsedData = tinyLoader->parseTextureData( meshPtr );
        m_parsedMaterialsQueue.push(
          ParsedMaterials{ .pMesh = meshPtr, .parsedMaterialData = std::move( parsedData ) } );
        enqueueMesh( meshPtr );
        m_readyForUpdate.store( true );
      }
    } );

  taskManager->addTaskSetToPipe( callbackPtr );

  return m_loadedMeshes[meshPath].get();
}

auto core::AssetManager::loadFont( const std::string_view path ) -> void
{
  KASSERT( std::filesystem::exists( path ) && "File does not exist" );
  ZoneScopedN( "AssetManager::loadFont" );
  m_fontLoader.generateAtlas( path );

  // now load the textures
  std::filesystem::path p{ path };
  auto fontName = p.stem().string();
  std::unique_ptr<resources::Font> font = std::make_unique<resources::Font>(
    fontName, p.parent_path().string() + "/" + fontName + ".json", p.parent_path().string() + "/" + fontName + ".png" );
  m_loadedFonts.emplace( p.parent_path().string(), std::move( font ) );
}

auto core::AssetManager::getMesh( const std::string& path ) -> resources::Mesh*
{
  std::filesystem::path p = std::filesystem::path{ path };
  if ( m_loadedMeshes.contains( path ) )
  {
    return m_loadedMeshes[path].get();
  }

  return loadMesh( p.stem().string(), p.string() );
}

auto core::AssetManager::createIndexBuffer( resources::Mesh* pMesh ) -> void
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

  graphics::VulkanBuffer stageBuffer = m_vkCtx->device->createStagingBuffer( stageBufferInfo, stageAllocInfo );
  m_vkCtx->device->copyMemoryToAllocation( indices.data(), stageBuffer.vmaAllocation, 0, bufferSize );

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;

  auto meshPath = std::filesystem::path{ pMesh->getPath() };
  auto name = std::string{ meshPath.stem().string() + "_indicesBuff" };

  m_vkCtx->device->createBuffer( pMesh->getIndicesBufferObject(), bufferInfo, vmaAllocInfo, name );
  m_vkCtx->device->copyBuffer( stageBuffer.vkBuffer, pMesh->getIndicesBufferObject().vkBuffer, bufferSize );
  m_vkCtx->device->destroyBuffer( stageBuffer );
}

auto core::AssetManager::createVertexBuffer( resources::Mesh* pMesh ) -> void
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

  graphics::VulkanBuffer stageBuffer = m_vkCtx->device->createStagingBuffer( stageBufferInfo, stageAllocInfo );

  m_vkCtx->device->copyMemoryToAllocation( vertices.data(), stageBuffer.vmaAllocation, 0, bufferSize );

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  std::filesystem::path meshPath = pMesh->getPath();
  std::string name = meshPath.stem().string() + "_verticesBuff";

  m_vkCtx->device->createBuffer( pMesh->getVertexBufferObject(), bufferInfo, vmaAllocInfo, name );
  m_vkCtx->device->copyBuffer( stageBuffer.vkBuffer, pMesh->getVertexBufferObject().vkBuffer, bufferSize );
  m_vkCtx->device->destroyBuffer( stageBuffer );
}

auto core::AssetManager::createBindlessDescriptorSetLayout() -> void
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
  layoutInfo.pNext = &bindingFlags;
  layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

  m_vkCtx->device->createDescriptorSetLayout( m_bindlessTexturesDescriptor.layout, layoutInfo );
}

auto core::AssetManager::allocateBindlessDescriptorSet() -> void
{
  uint32_t descriptorCount{ MAX_TEXTURE_SUPPORT };

  VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
  variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
  variableCountInfo.descriptorSetCount = 1;
  variableCountInfo.pDescriptorCounts = &descriptorCount;

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

  allocInfo.descriptorPool = m_vkCtx->globalDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_bindlessTexturesDescriptor.layout;
  allocInfo.pNext = &variableCountInfo;

  m_vkCtx->device->allocateDescriptorSet( m_bindlessTexturesDescriptor.set, allocInfo );
}

auto core::AssetManager::updateBindlessTextures( const std::vector<resources::Texture*>& textures ) -> void
{
  if ( m_textureSampler == VK_NULL_HANDLE )
  {
    throw std::runtime_error( "sampler is not initialized" );
  }

  std::vector<VkWriteDescriptorSet> descriptorWrites{};
  std::vector<VkDescriptorImageInfo> imageInfo{};

  for ( resources::Texture* tex : textures )
  {
    imageInfo.push_back( VkDescriptorImageInfo{
      .sampler = m_textureSampler,
      .imageView = tex->getView(),
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    } );

    descriptorWrites.push_back( VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = m_bindlessTexturesDescriptor.set,
      .dstBinding = 0,
      .dstArrayElement = m_bindlessTexturesIndex,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      // careful with this, not safe
      .pImageInfo = &imageInfo[std::size( imageInfo ) - 1],
    } );

    tex->setIndex( m_bindlessTexturesIndex );

    ++m_bindlessTexturesIndex;
  }

  m_vkCtx->device->updateDescriptorSet( descriptorWrites );
}

auto core::AssetManager::updateBindlessTextures( resources::Texture* pTexture ) -> void
{
  if ( m_textureSampler == VK_NULL_HANDLE )
  {
    throw std::runtime_error( "sampler is not initialized" );
  }

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = pTexture->getView();
  imageInfo.sampler = m_textureSampler;

  VkWriteDescriptorSet bindlessDescriptor{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                           .dstSet = m_bindlessTexturesDescriptor.set,
                                           .dstBinding = 0,
                                           .dstArrayElement = m_bindlessTexturesIndex,
                                           .descriptorCount = 1,
                                           .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                           .pImageInfo = &imageInfo };

  pTexture->setIndex( m_bindlessTexturesIndex );
  ++m_bindlessTexturesIndex;

  m_vkCtx->device->updateDescriptorSet( { bindlessDescriptor } );
}

auto core::AssetManager::createMaterialsDescriptorSet() -> void
{
  uint32_t descriptorCount{ 1000 };

  VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
  variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
  variableCountInfo.descriptorSetCount = 1;
  variableCountInfo.pDescriptorCounts = &descriptorCount;

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

  allocInfo.descriptorPool = m_vkCtx->globalDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_materialsDescriptor.layout;
  allocInfo.pNext = &variableCountInfo;

  m_vkCtx->device->allocateDescriptorSet( m_materialsDescriptor.set, allocInfo );
}

auto core::AssetManager::createMaterialsDescriptorSetLayout() -> void
{
  VkDescriptorSetLayoutBinding bufferLayoutBinding{};
  bufferLayoutBinding.binding = 0;
  bufferLayoutBinding.descriptorCount = 1000;
  bufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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
  layoutInfo.pBindings = &bufferLayoutBinding;
  layoutInfo.pNext = &bindingFlags;
  layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

  m_vkCtx->device->createDescriptorSetLayout( m_materialsDescriptor.layout, layoutInfo );
}

auto core::AssetManager::allocateMaterialsDescriptorSet() -> void
{
  uint32_t descriptorCount{ 1000 };

  VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
    .descriptorSetCount = 1,
    .pDescriptorCounts = &descriptorCount };

  VkDescriptorSetAllocateInfo allocInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext = &variableCountInfo,
    .descriptorPool = m_vkCtx->globalDescriptorPool,
    .descriptorSetCount = 1,
    .pSetLayouts = &m_materialsDescriptor.layout,
  };

  m_vkCtx->device->allocateDescriptorSet( m_materialsDescriptor.set, allocInfo );
}

auto core::AssetManager::createMaterialsBuffers( VkDeviceSize size ) -> void
{
  VkBufferCreateInfo bufferInfo{
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT };

  VmaAllocationCreateInfo vmaAllocInfo{
    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
  };

  m_vkCtx->device->createBuffer( m_materialsBuffer, bufferInfo, vmaAllocInfo, "materialsBuffer" );
}

auto core::AssetManager::updateMaterialsBuffer() -> void
{
  uint32_t currentFrame = m_vkCtx->swapchain->getCurrentFrameNumber();

  VkDescriptorBufferInfo buffInfo{
    .buffer = m_materialsBuffer.buffers.at( currentFrame ).vkBuffer,
    .offset = 0,
    .range = m_materials.size() * sizeof( resources::Material ),
  };

  VkWriteDescriptorSet bindlessDescriptor{
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = m_materialsDescriptor.set,
    .dstBinding = 0,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    .pBufferInfo = &buffInfo,
  };

  m_vkCtx->device->copyToBuffer( m_materials, m_materialsBuffer.buffers.at( currentFrame ) );
  m_vkCtx->device->updateDescriptorSet( { bindlessDescriptor } );
}

auto core::AssetManager::getMaterialsDescriptorLayout() -> VkDescriptorSetLayout&
{
  return m_materialsDescriptor.layout;
}

auto core::AssetManager::getMaterialsDescriptorSet() -> VkDescriptorSet&
{
  return m_materialsDescriptor.set;
}

auto core::AssetManager::getBindlessTexturesDescriptorLayout() -> VkDescriptorSetLayout&
{
  return m_bindlessTexturesDescriptor.layout;
}

auto core::AssetManager::getBindlessDescriptorSet() -> VkDescriptorSet&
{
  return m_bindlessTexturesDescriptor.set;
}

auto core::AssetManager::getMeshes() -> std::unordered_map<std::string, std::unique_ptr<resources::Mesh>>&
{
  return m_loadedMeshes;
}

auto core::AssetManager::getTextures() -> std::unordered_map<std::string, std::unique_ptr<resources::Texture>>&
{
  return m_loadedTextures;
}

auto core::AssetManager::uploadMeshData() -> void
{
  while ( !m_queuedMeshes.empty() )
  {
    resources::Mesh* mesh = m_queuedMeshes.front();
    m_queuedMeshes.pop();

    if ( !mesh->isLoaded() )
    {
      createMeshResources( mesh );
      mesh->setLoaded( true );
      KINFO( "Loaded {}", mesh->getPath() );
    }
  }
}

auto core::AssetManager::onUpdate() -> void
{
  if ( !m_readyForUpdate.load() )
  {
    return;
  }

  m_readyForUpdate.store( false );

  std::vector<ParsedMaterials> batch;
  ZoneScopedN( "AssetManager::OnUpdate" );
  {
    std::lock_guard lock( m_mutex );
    while ( !m_parsedMaterialsQueue.empty() )
    {
      batch.push_back( std::move( m_parsedMaterialsQueue.front() ) );
      m_parsedMaterialsQueue.pop();
    }
  }

  std::set<std::string> needed;
  for ( auto& entry : batch )
  {
    for ( auto& [submesh, textureMap] : entry.parsedMaterialData )
    {
      for ( auto type : { Diffuse, Emissive, Normal } )
      {
        if ( !textureMap.at( type ).empty() )
        {
          needed.insert( textureMap.at( type ) );
        }
      }
    }
  }

  std::vector<std::tuple<std::string, std::string>> toLoad;
  for ( auto& path : needed )
  {
    if ( !getTexture( path ) )
    {
      std::filesystem::path p{ path };
      toLoad.emplace_back( p.string(), p.stem().string() );
    }
  }

  if ( !toLoad.empty() )
  {
    loadTextures( toLoad );
  }

  bool materialsChanged{ false };
  for ( auto& entry : batch )
  {
    for ( auto& [submesh, textureMap] : entry.parsedMaterialData )
    {
      resources::Material material{};
      auto& submeshes = entry.pMesh->getSubmeshes();

      resources::Texture* diffuse = getTexture( textureMap.at( Diffuse ) );
      if ( diffuse )
      {
        material.diffuseTextureIndex = diffuse->getIndex();
      }

      resources::Texture* emissive = getTexture( textureMap.at( Emissive ) );
      if ( emissive )
      {
        material.emissiveTextureIndex = emissive->getIndex();
      }

      resources::Texture* normal = getTexture( textureMap.at( Normal ) );
      if ( normal )
      {
        material.normalTextureIndex = normal->getIndex();
      }

      resources::Material stale{};
      if ( material == stale )
      {
        submeshes[submesh].materialIndex = 0;
        KINFO( "default material" );
        continue;
      }

      auto it = std::ranges::find( m_materials, material );
      if ( it != m_materials.end() )
      {
        submeshes[submesh].materialIndex = std::distance( m_materials.begin(), it );
        KINFO( "material already loaded, index {}", submeshes[submesh].materialIndex );
      }
      else
      {
        submeshes[submesh].materialIndex = m_materials.size();
        KINFO( "new material at index {}", m_materials.size() );
        m_materials.push_back( material );
        materialsChanged = true;
      }
    }

    if ( materialsChanged )
    {
      updateMaterialsBuffer();
    }
  }

  if ( !m_queuedMeshes.empty() )
  {
    uploadMeshData();
  }
}

auto core::AssetManager::createMeshResources( resources::Mesh* pMesh ) -> void
{
  createVertexBuffer( pMesh );
  createIndexBuffer( pMesh );
}

auto core::AssetManager::enqueueMesh( resources::Mesh* mesh ) -> void
{
  m_queuedMeshes.push( mesh );
}
