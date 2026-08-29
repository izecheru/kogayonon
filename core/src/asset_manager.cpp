#include "core/asset_manager/asset_manager.hpp"
#include "resources/font.hpp"
#include <vulkan/vulkan.h>
#define STB_IMAGE_IMPLEMENTATION
#include "graphics/utils.hpp"
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "resources/mesh.hpp"
#include "resources/texture.hpp"
#include "utilities/tracy_utils/tracy_utils.hpp"
#include "utilities/utils/utils.hpp"
#include <stb_image.h>

core::AssetManager::AssetManager()
    : m_bindlessTexturesIndex{ 0u }
    , m_bindlessMaterialIndex{ 0u }
    , m_samplerIndex{ 0u }
{
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
    return m_loadedTextures[texturePath].get();

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
  stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

  VmaAllocationCreateInfo stageAllocInfo{};
  stageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

  graphics::VulkanBuffer stageBuffer = m_vkCtx->device->createStagingBuffer( stageBufferInfo, stageAllocInfo );

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
  imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;

  m_vkCtx->device->createImage( texture->getImage(), imageInfo, imageAllocInfo, texture->getAllocation(), textureName );

  m_vkCtx->device->createImageView(
    texture->getView(), texture->getImage(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

  // transfer barrier
  m_vkCtx->device->transitionImageLayout( texture->getImage(),
                                          { .srcStage = VK_PIPELINE_STAGE_2_NONE,
                                            .currentStage = VK_PIPELINE_STAGE_2_COPY_BIT,
                                            .srcAccess = VK_ACCESS_2_NONE,
                                            .currentAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                            .currentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL } );

  void* data;
  vmaMapMemory( m_vkCtx->device->getAllocator(), stageBuffer.vmaAllocation, &data );
  memcpy( data, pixels, static_cast<size_t>( imageSize ) );
  vmaUnmapMemory( m_vkCtx->device->getAllocator(), stageBuffer.vmaAllocation );

  stbi_image_free( pixels );

  // copy and also transition layout
  m_vkCtx->device->copyBufferToImage( stageBuffer.vkBuffer,
                                      texture->getImage(),
                                      static_cast<uint32_t>( texWidth ),
                                      static_cast<uint32_t>( texHeight ),
                                      { .srcStage = VK_PIPELINE_STAGE_2_COPY_BIT,
                                        .currentStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                        .srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                        .currentAccess = VK_ACCESS_2_SHADER_READ_BIT,
                                        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        .currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } );

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
    createMaterialsBuffers( sizeof( resources::Material ) );
    createMaterialsDescriptorSetLayout();
    allocateMaterialsDescriptorSet();

    m_layoutInit = true;
  }
}

auto core::AssetManager::setContext( graphics::VulkanContext* ctx ) -> void
{
  m_vkCtx = ctx;
}

auto core::AssetManager::getTexture( const std::string& texturePath ) -> resources::Texture*
{
  std::filesystem::path p{ texturePath };

  if ( m_loadedTextures.contains( texturePath ) )
    return m_loadedTextures[texturePath].get();

  return loadTexture( p.filename().string(), texturePath );
}

auto core::AssetManager::initSampler() -> void
{
  m_vkCtx->device->createSampler( m_textureSampler );
  ++m_samplerIndex;
}

auto core::AssetManager::loadMesh( const std::string& meshName, const std::string& meshPath ) -> resources::Mesh*
{
  ZoneScopedN( "AssetManager::loadMesh" );
  assert( std::filesystem::exists( meshPath ) && "mesh file does not exist" );

  if ( m_loadedMeshes.contains( meshPath ) )
    return m_loadedMeshes[meshPath].get();

  std::unique_ptr<resources::Mesh> mesh = std::make_unique<resources::Mesh>();

  mesh->setPath( meshPath );
  std::filesystem::path p{ meshPath };

  std::vector<aiMaterial*> materials{};
  m_assimpLoader.loadMesh( meshPath, mesh.get(), materials );
  std::vector<resources::Submesh>& submeshes = mesh->getSubmeshes();

  if ( m_materials.empty() )
  {
    resources::Material defaultMaterial{};

    std::filesystem::path path = std::filesystem::current_path() / "engine_resources" / "textures" / "default.png";

    if ( std::filesystem::exists( path ) && !m_loadedTextures.contains( meshPath ) )
    {
      auto texture = loadTexture( "default", path.string() );
      texture->setIndex( m_bindlessTexturesIndex );
      defaultMaterial.diffuseTextureIndex = m_bindlessTexturesIndex;
      updateBindlessTextures( texture );
    }

    m_materials.push_back( defaultMaterial );
    updateMaterialsBuffer();
  }

  for ( auto i = 0u; i < materials.size(); i++ )
  {

    resources::Material mat{};
    aiMaterial* material = materials.at( i );

    if ( material->GetTextureCount( aiTextureType_EMISSIVE ) > 0 )
    {
      aiString str;
      material->GetTexture( aiTextureType_EMISSIVE, 0, &str );

      std::filesystem::path path =
        std::filesystem::absolute( "." ) / ( "engine_resources\\" + std::string( str.C_Str() ) );

      std::string key = std::filesystem::weakly_canonical( path ).string();
      if ( !m_loadedTextures.contains( key ) )
      {
        K_INFO( "tex {} index {}", key, m_bindlessTexturesIndex );
        mat.emissiveTextureIndex = m_bindlessTexturesIndex;
        auto texture = loadTexture( path.stem().string(), key );
        texture->setIndex( m_bindlessTexturesIndex );
        updateBindlessTextures( texture );
      }
      else
      {
        mat.emissiveTextureIndex = getTexture( key )->getIndex();
      }
    }

    if ( material->GetTextureCount( aiTextureType_SPECULAR ) > 0 )
    {
      aiString str;
      material->GetTexture( aiTextureType_SPECULAR, 0, &str );

      std::filesystem::path path =
        std::filesystem::absolute( "." ) / ( "engine_resources\\" + std::string( str.C_Str() ) );

      std::string key = std::filesystem::weakly_canonical( path ).string();
      if ( !m_loadedTextures.contains( key ) )
      {
        K_INFO( "tex {} index {}", key, m_bindlessTexturesIndex );
        mat.specularTextureIndex = m_bindlessTexturesIndex;
        auto texture = loadTexture( path.stem().string(), key );
        texture->setIndex( m_bindlessTexturesIndex );
        updateBindlessTextures( texture );
      }
      else
      {
        mat.specularTextureIndex = getTexture( key )->getIndex();
      }
    }

    if ( material->GetTextureCount( aiTextureType_NORMALS ) > 0 )
    {
      aiString str;
      material->GetTexture( aiTextureType_NORMALS, 0, &str );

      std::filesystem::path path =
        std::filesystem::absolute( "." ) / ( "engine_resources\\" + std::string( str.C_Str() ) );

      std::string key = std::filesystem::weakly_canonical( path ).string();

      if ( !m_loadedTextures.contains( key ) )
      {
        K_INFO( "tex {} index {}", key, m_bindlessTexturesIndex );
        mat.normalTextureIndex = m_bindlessTexturesIndex;
        auto texture = loadTexture( path.stem().string(), key );
        texture->setIndex( m_bindlessTexturesIndex );
        updateBindlessTextures( texture );
      }
      else
      {
        mat.normalTextureIndex = getTexture( key )->getIndex();
      }
    }

    if ( material->GetTextureCount( aiTextureType_DIFFUSE ) > 0 )
    {
      aiString str;
      material->GetTexture( aiTextureType_DIFFUSE, 0, &str );

      std::filesystem::path path =
        std::filesystem::absolute( "." ) / ( "engine_resources\\" + std::string( str.C_Str() ) );

      std::string key = std::filesystem::weakly_canonical( path ).string();
      if ( !m_loadedTextures.contains( key ) )
      {
        K_INFO( "tex {} index {}", key, m_bindlessTexturesIndex );
        mat.diffuseTextureIndex = m_bindlessTexturesIndex;
        auto texture = loadTexture( path.stem().string(), key );
        texture->setIndex( m_bindlessTexturesIndex );
        updateBindlessTextures( texture );
      }
      else
      {
        mat.diffuseTextureIndex = getTexture( key )->getIndex();
      }
    }

    int32_t foundIndex{ -1 };
    for ( auto j = 0u; j < m_materials.size(); ++j )
    {
      resources::Material& material = m_materials.at( j );
      if ( material.diffuseTextureIndex == mat.diffuseTextureIndex &&
           material.normalTextureIndex == mat.normalTextureIndex &&
           material.specularTextureIndex == mat.specularTextureIndex )
      {
        if ( material.diffuseTextureIndex != -1 && material.specularTextureIndex != -1 &&
             material.normalTextureIndex != -1 )
        {
          foundIndex = j;
          break;
        }
      }
    }

    if ( foundIndex == -1 )
    {
      m_materials.push_back( mat );
      submeshes[i].materialIndex = m_materials.size() - 1;
      updateMaterialsBuffer();
    }
    else
    {
      submeshes[i].materialIndex = foundIndex;
    }
  }

  createVertexBuffer( mesh.get() );
  createIndexBuffer( mesh.get() );

  m_loadedMeshes.emplace( meshPath, std::move( mesh ) );
  return m_loadedMeshes[meshPath].get();
}

auto core::AssetManager::loadFont( const std::string_view path ) -> void
{
  K_ASSERT( std::filesystem::exists( path ) && "File does not exist" );
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
    return m_loadedMeshes[path].get();

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

  allocInfo.descriptorPool = m_pDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_bindlessTexturesDescriptor.layout;
  allocInfo.pNext = &variableCountInfo;

  m_vkCtx->device->allocateDescriptorSet( m_bindlessTexturesDescriptor.set, allocInfo );
}

auto core::AssetManager::setDescriptorPool( VkDescriptorPool pool ) -> void
{
  m_pDescriptorPool = pool;
}

auto core::AssetManager::updateBindlessTextures( resources::Texture* pTexture ) -> void
{
  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = pTexture->getView();
  imageInfo.sampler = m_textureSampler;

  auto bindlessDescriptor = VkWriteDescriptorSet{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
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

  allocInfo.descriptorPool = m_pDescriptorPool;
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

  VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
  variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
  variableCountInfo.descriptorSetCount = 1;
  variableCountInfo.pDescriptorCounts = &descriptorCount;

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

  allocInfo.descriptorPool = m_pDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_materialsDescriptor.layout;
  allocInfo.pNext = &variableCountInfo;

  m_vkCtx->device->allocateDescriptorSet( m_materialsDescriptor.set, allocInfo );
}

auto core::AssetManager::createMaterialsBuffers( VkDeviceSize size ) -> void
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  m_vkCtx->device->createBuffer( m_materialsBuffer, bufferInfo, vmaAllocInfo, "materialsBuffer" );
}

auto core::AssetManager::updateMaterialsBuffer() -> void
{

  m_vkCtx->device->destroyBuffer( m_materialsBuffer );

  VkDeviceSize totalSize = m_materials.size() * sizeof( resources::Material );

  createMaterialsBuffers( totalSize );

  VkDescriptorBufferInfo buffInfo{};
  buffInfo.buffer = m_materialsBuffer.vkBuffer;
  buffInfo.offset = 0;
  buffInfo.range = VK_WHOLE_SIZE;

  auto bindlessDescriptor = VkWriteDescriptorSet{
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = m_materialsDescriptor.set,
    .dstBinding = 0,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    .pBufferInfo = &buffInfo,
  };

  ++m_bindlessMaterialIndex;
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

auto core::AssetManager::getBindlessDescriptorLayout() -> VkDescriptorSetLayout&
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
