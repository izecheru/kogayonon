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

auto core::AssetManager::loadTexture( const std::string& textureName, const std::string& texturePath )
  -> resources::Texture*
{
  assert( std::filesystem::exists( texturePath ) && "texture file MUST EXIST" );
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

  VkBufferCreateInfo stageBufferInfo{};
  stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  stageBufferInfo.size = imageSize;
  stageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  stageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo stageAllocInfo{};
  stageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

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

  void* data;
  vmaMapMemory( m_vkCtx->device->getAllocator(), stageBuffer.vmaAllocation, &data );
  memcpy( data, pixels, static_cast<size_t>( imageSize ) );
  vmaUnmapMemory( m_vkCtx->device->getAllocator(), stageBuffer.vmaAllocation );

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
    createMaterialsBuffers( 500 * sizeof( resources::Material ) );
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
      auto tinyLoader = std::make_unique<TinyGltfLoader>( meshPath );
      std::lock_guard lock( m_mutex );
      tinyLoader->processVertexData( meshPtr );
      m_parsedMaterials.parsedMaterialData = std::move( tinyLoader->parseTextureData( meshPtr ) );
      m_parsedMaterials.pMesh = meshPtr;
      enqueueMesh( meshPtr );
    } );

  // this will run ONLY after the task above finished
  utilities::CallbackTask* flagReady =
    taskManager->addTask( [this, meshPtr = m_loadedMeshes[meshPath].get()]() -> void {
      std::lock_guard lock( m_mutex );
      m_readyForUpdate.store( true );
    } );

  taskManager->addDependency( flagReady, callbackPtr );
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

  auto stageBuffer = m_vkCtx->device->createStagingBuffer( stageBufferInfo, stageAllocInfo );

  void* data;
  vmaMapMemory( m_vkCtx->device->getAllocator(), stageBuffer.vmaAllocation, &data );
  memcpy( data, indices.data(), (size_t)bufferSize );
  vmaUnmapMemory( m_vkCtx->device->getAllocator(), stageBuffer.vmaAllocation );

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

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

  auto stageBuffer = m_vkCtx->device->createStagingBuffer( stageBufferInfo, stageAllocInfo );

  void* data;
  vmaMapMemory( m_vkCtx->device->getAllocator(), stageBuffer.vmaAllocation, &data );
  memcpy( data, vertices.data(), (size_t)bufferSize );
  vmaUnmapMemory( m_vkCtx->device->getAllocator(), stageBuffer.vmaAllocation );

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  auto meshPath = std::filesystem::path{ pMesh->getPath() };
  auto name = std::string{ meshPath.stem().string() + "_verticesBuff" };
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

  ++m_bindlessTexturesIndex;

  m_vkCtx->device->updateDescriptorSet( { bindlessDescriptor } );
}

auto core::AssetManager::createMaterialsDescriptorSet() -> void
{
  uint32_t descriptorCount{ 500 };

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
  bufferLayoutBinding.descriptorCount = 500;
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
  uint32_t descriptorCount{ 500 };

  VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
    .descriptorSetCount = 1,
    .pDescriptorCounts = &descriptorCount };

  VkDescriptorSetAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };

  allocInfo.descriptorPool = m_vkCtx->globalDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_materialsDescriptor.layout;
  allocInfo.pNext = &variableCountInfo;

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
  m_vkCtx->device->destroyBuffer( m_materialsBuffer );
  createMaterialsBuffers( m_materials.size() * sizeof( resources::Material ) );

  VkDescriptorBufferInfo buffInfo{
    .buffer = m_materialsBuffer.vkBuffer,
    .offset = 0,
    .range = VK_WHOLE_SIZE,
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

  m_vkCtx->device->copyToBuffer( m_materials, m_materialsBuffer );
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

  if ( !m_queuedMeshes.empty() )
  {
    uploadMeshData();
  }

  if ( !m_parsedMaterials.parsedMaterialData.empty() )
  {
    resources::Material staleMaterial{};
    resources::Mesh* mesh = m_parsedMaterials.pMesh;
    std::vector<resources::Submesh>& submeshes = mesh->getSubmeshes();
    for ( const auto& [submeshIndex, textureMap] : m_parsedMaterials.parsedMaterialData )
    {
      using enum TextureType;
      resources::Material material{};
      if ( !textureMap.at( Diffuse ).empty() )
      {
        std::filesystem::path texturePath{ textureMap.at( Diffuse ) };
        if ( !getTexture( texturePath.string() ) )
        {
          resources::Texture* texture = loadTexture( texturePath.stem().string(), texturePath.string() );
          texture->setIndex( m_bindlessTexturesIndex );
          material.diffuseTextureIndex = texture->getIndex();
          updateBindlessTextures( texture );
        }
        else
        {
          resources::Texture* texture = getTexture( texturePath.string() );
          material.diffuseTextureIndex = texture->getIndex();
        }
      }

      if ( !textureMap.at( Emissive ).empty() )
      {
        std::filesystem::path texturePath{ textureMap.at( Emissive ) };
        if ( !getTexture( texturePath.string() ) )
        {
          resources::Texture* texture = loadTexture( texturePath.stem().string(), texturePath.string() );
          texture->setIndex( m_bindlessTexturesIndex );
          material.emissiveTextureIndex = texture->getIndex();
          updateBindlessTextures( texture );
        }
        else
        {
          resources::Texture* texture = getTexture( texturePath.string() );
          material.emissiveTextureIndex = texture->getIndex();
        }
      }

      if ( !textureMap.at( Normal ).empty() )
      {
        std::filesystem::path texturePath{ textureMap.at( Normal ) };
        if ( !getTexture( texturePath.string() ) )
        {
          resources::Texture* texture = loadTexture( texturePath.stem().string(), texturePath.string() );
          texture->setIndex( m_bindlessTexturesIndex );
          material.normalTextureIndex = texture->getIndex();
          updateBindlessTextures( texture );
        }
        else
        {
          resources::Texture* texture = getTexture( texturePath.string() );
          material.normalTextureIndex = texture->getIndex();
        }
      }

      // if we have no texture loaded then this is the default matrial
      if ( material == staleMaterial )
      {
        submeshes[submeshIndex].materialIndex = 0;
        KINFO( "using default material (index 0)" );
        continue;
      }

      auto it = std::ranges::find( m_materials, material );
      if ( it != m_materials.end() )
      {
        submeshes[submeshIndex].materialIndex = static_cast<uint32_t>( std::distance( m_materials.begin(), it ) );
        KINFO( "using index {}", submeshes[submeshIndex].materialIndex );
      }
      else
      {
        submeshes[submeshIndex].materialIndex = m_materials.size();
        m_materials.push_back( material );
        KINFO( "created material, index is size-1={}", submeshes[submeshIndex].materialIndex );
        updateMaterialsBuffer();
      }
    }
    m_parsedMaterials.parsedMaterialData.clear();
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
