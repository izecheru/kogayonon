#include <vulkan/vulkan_core.h>
#include "resources/mesh.hpp"
#include "core/asset_manager/ktx_texture_manager.hpp"
#include "resources/texture.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "resources/font.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "utilities/utils/utils.hpp"

#include "utilities/tracy_utils/tracy_utils.hpp"

core::AssetManager::AssetManager( graphics::VulkanContext* vkCtx )
    : m_bindlessTexturesIndex{ 0u }
    , m_samplerIndex{ 0u }
    , m_vkCtx{ vkCtx }
    , m_ktxTextureManager{ std::make_unique<KtxTextureManager>( vkCtx->device.get() ) }
//, m_threadSafeResourceManager{ 4, m_vkCtx->device.get() }
{
    init();
}

auto core::AssetManager::recreate() -> void
{
    m_vkCtx->device->waitIdle();
    m_bindlessTexturesIndex = 0u;

    for ( auto& [path, texture] : m_loadedTextures )
    {
        auto it = std::ranges::find_if( m_uiTexturePaths, [&]( const std::filesystem::path& p ) { return p == path; } );

        if ( it != m_uiTexturePaths.end() )
        {
            continue;
        }

        m_vkCtx->device->destroyImageView( texture->vulkanImage.vkImageView );
        m_vkCtx->device->destroyImage( texture->vulkanImage.vkImage, texture->vulkanImage.vmaAllocation );
    }

    for ( auto& [path, mesh] : m_loadedMeshes )
    {
        m_vkCtx->device->destroyBuffer( mesh->getIndicesBufferObject() );
        m_vkCtx->device->destroyBuffer( mesh->getVertexBufferObject() );
    }

    std::erase_if( m_loadedTextures, [&]( const auto& kv ) {
        return std::ranges::find( m_uiTexturePaths, std::filesystem::path{ kv.first } ) == m_uiTexturePaths.end();
    } );

    m_loadedMeshes.clear();
    m_materials.clear();
}

auto core::AssetManager::init() -> void
{
    initDescriptors();
    initSampler();

    VkSemaphoreTypeCreateInfo meshTimelineCreateInfo{};
    meshTimelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    meshTimelineCreateInfo.pNext = NULL;
    meshTimelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    meshTimelineCreateInfo.initialValue = 0;

    VkSemaphoreCreateInfo meshCreateInfo{};
    meshCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    meshCreateInfo.pNext = &meshTimelineCreateInfo;
    meshCreateInfo.flags = 0;

    m_vkCtx->device->createSemaphore( m_meshTimelineSemaphore, meshTimelineCreateInfo, meshCreateInfo );

    VkSemaphoreTypeCreateInfo timelineCreateInfo;
    timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timelineCreateInfo.pNext = NULL;
    timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineCreateInfo.initialValue = 0;

    VkSemaphoreCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = &timelineCreateInfo;
    createInfo.flags = 0;

    m_vkCtx->device->createSemaphore( m_textureTimelineSemaphore, timelineCreateInfo, createInfo );
}

auto core::AssetManager::destroyResources() -> void
{
    m_vkCtx->device->waitIdle();

    m_vkCtx->device->destroyBuffer( m_materialsBuffer );
    m_vkCtx->device->destroySemaphore( m_textureTimelineSemaphore );
    m_vkCtx->device->destroySemaphore( m_meshTimelineSemaphore );
    m_vkCtx->device->destroySampler( m_textureSampler );
    m_vkCtx->device->destroyDescriptorSetLayout( m_bindlessTexturesDescriptor.layout );
    m_vkCtx->device->destroyDescriptorSetLayout( m_materialsDescriptor.layout );
    m_bindlessTexturesIndex = 0u;

    for ( auto& [path, texture] : m_loadedTextures )
    {
        m_vkCtx->device->destroyImageView( texture->vulkanImage.vkImageView );
        m_vkCtx->device->destroyImage( texture->vulkanImage.vkImage, texture->vulkanImage.vmaAllocation );
    }

    for ( auto& [path, mesh] : m_loadedMeshes )
    {
        m_vkCtx->device->destroyBuffer( mesh->getIndicesBufferObject() );
        m_vkCtx->device->destroyBuffer( mesh->getVertexBufferObject() );
    }

    m_loadedTextures.clear();
    m_loadedMeshes.clear();
    m_materials.clear();
}

auto core::AssetManager::shutdown() -> void
{
    if ( m_shutdown.exchange( true ) )
    {
        return;
    }

    m_acceptingLoads = false;

    utilities::TaskManager* taskManager = core::MainRegistry::getInstance().getTaskManager();

    if ( !taskManager )
    {
        return;
    }

    for ( enki::ITaskSet* taskSet : m_meshLoadTasks )
    {
        if ( !taskSet->GetIsComplete() )
        {
            taskManager->getScheduler().WaitforTask( taskSet );
        }
    }

    for ( enki::ITaskSet* taskSet : m_textureLoadTasks )
    {
        if ( !taskSet->GetIsComplete() )
        {
            taskManager->getScheduler().WaitforTask( taskSet );
        }
    }

    m_vkCtx->device->waitIdle();

    for ( enki::ITaskSet* taskSet : m_meshLoadTasks )
    {
        auto* task = static_cast<utilities::DataTaskSet<MeshTaskData>*>( taskSet );

        m_vkCtx->device->destroyBuffer( task->container.stageBuffer );
        taskManager->eraseTask( taskSet, utilities::TaskType::Data );
    }

    for ( enki::ITaskSet* taskSet : m_textureLoadTasks )
    {
        auto* task = static_cast<utilities::DataTaskSet<TextureTaskData>*>( taskSet );

        m_vkCtx->device->destroyBuffer( task->container.stagingBuffer );

        for ( ImageData& imageData : task->container.imageData )
        {
            m_vkCtx->device->destroyImage( imageData.texture->vulkanImage.vkImage,
                                           imageData.texture->vulkanImage.vmaAllocation );

            m_vkCtx->device->destroyImageView( imageData.texture->vulkanImage.vkImageView );
        }

        taskManager->eraseTask( taskSet, utilities::TaskType::Data );
    }

    m_meshLoadTasks.clear();
    m_textureLoadTasks.clear();
    m_meshTaskSync.clear();
    m_textureTaskSync.clear();

    destroyResources();
}

core::AssetManager::~AssetManager()
{
    shutdown();
}

auto core::AssetManager::loadTextureData() -> void
{
    utilities::TaskManager* taskManager = core::MainRegistry::getInstance().getTaskManager();

    std::vector<ParsedMaterials> batch{};
    while ( !m_parsedMaterialsQueue.empty() )
    {
        batch.emplace_back( m_parsedMaterialsQueue.front() );
        m_parsedMaterialsQueue.pop();
    }

    for ( ParsedMaterials& b : batch )
    {
        utilities::TaskSet* generateKtx = taskManager->addTask( [this, b]() {
            ZoneScopedN( "[TASK] generateKtx" );
            for ( auto& [submesh, textureMap] : b.parsedMaterialData )
            {
                for ( auto type : { Diffuse, Emissive, Normal } )
                {
                    if ( !textureMap.at( type ).empty() )
                    {
                        std::filesystem::path p{ textureMap.at( type ) };
                        std::string ktxFilename = p.stem().string() + ".ktx2";
                        std::filesystem::path ktxPath = p.parent_path() / ktxFilename;

                        if ( !hasResource<resources::Texture>( ktxPath.string() ) )
                        {
                            bool ktxExists = std::filesystem::exists( ktxPath );

                            if ( !ktxExists )
                            {
                                m_ktxTextureManager->saveToKtx( p );
                            }
                        }
                    }
                }
            }
        } );

        utilities::DataTaskSet<TextureTaskData>* task =
            taskManager->addTask( TextureTaskData{}, [this, b]( core::TextureTaskData& container ) {
                ZoneScopedN( "[TASK] loadTextureData" );
                std::set<std::string> needed;
                for ( auto& [submesh, textureMap] : b.parsedMaterialData )
                {
                    for ( auto type : { Diffuse, Emissive, Normal } )
                    {
                        if ( !textureMap.at( type ).empty() )
                        {
                            needed.insert( textureMap.at( type ) );
                        }
                    }
                }

                std::vector<std::tuple<std::string, std::string>> toLoad;
                for ( auto& path : needed )
                {
                    std::filesystem::path p{ path };
                    std::string ktxFilename = p.stem().string() + ".ktx2";
                    std::filesystem::path ktxPath = p.parent_path() / ktxFilename;

                    if ( !hasResource<resources::Texture>( ktxPath.string() ) &&
                         !m_pendingTextureLoads.contains( ktxPath.string() ) )
                    {
                        {
                            std::lock_guard lock{ m_materialMutex };
                            m_pendingTextureLoads.insert( ktxPath.string() );
                        }
                        std::filesystem::path p{ path };
                        toLoad.emplace_back( p.string(), p.stem().string() );
                    }
                }

                if ( toLoad.empty() )
                {
                    return;
                }

                std::vector<ImageData>& imageData = container.imageData;
                std::vector<VkImageMemoryBarrier2>& beforeBarriers = container.beforeBarriers;
                std::vector<VkImageMemoryBarrier2>& afterBarriers = container.afterBarriers;
                VkDeviceSize& stageBufferSize = container.stageBufferSize;

                container.materialsBatch = b;

                imageData.reserve( toLoad.size() );

                for ( const auto& [path, name] : toLoad )
                {
                    std::filesystem::path p{ path };
                    std::string ktxFilename = p.stem().string() + ".ktx2";
                    std::filesystem::path ktxPath = p.parent_path() / ktxFilename;

                    ImageData img{ .texture = std::make_unique<resources::Texture>() };

                    {
                        ZoneScopedN( "ktx load texture" );
                        if ( m_ktxTextureManager->loadTexture( ktxPath, img.texture.get() ) != KTX_SUCCESS )
                        {
                            continue;
                        }
                    }

                    img.data = m_ktxTextureManager->getTextureData( img.texture.get() );
                    img.size = m_ktxTextureManager->getDataSize( img.texture.get() );
                    img.w = img.texture->ktxImage.texture->baseWidth;
                    img.h = img.texture->ktxImage.texture->baseHeight;

                    stageBufferSize += img.size;

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

                    m_vkCtx->device->createImage( img.texture->vulkanImage.vkImage,
                                                  imageInfo,
                                                  imageAllocInfo,
                                                  img.texture->vulkanImage.vmaAllocation,
                                                  name );

                    m_vkCtx->device->createImageView( img.texture->vulkanImage.vkImageView,
                                                      img.texture->vulkanImage.vkImage,
                                                      VK_FORMAT_R8G8B8A8_UNORM,
                                                      VK_IMAGE_ASPECT_COLOR_BIT );

                    beforeBarriers.emplace_back(
                        VkImageMemoryBarrier2{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                               .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                                               .srcAccessMask = VK_ACCESS_2_NONE,
                                               .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                                               .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                               .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                               .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                               .image = img.texture->vulkanImage.vkImage,
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

                    afterBarriers.emplace_back(
                        VkImageMemoryBarrier2{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                               .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                                               .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                               .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                               .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                                               .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                               .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                               .image = img.texture->vulkanImage.vkImage,
                                               .subresourceRange = {
                                                   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                   .baseMipLevel = 0,
                                                   .levelCount = 1,
                                                   .baseArrayLayer = 0,
                                                   .layerCount = 1,
                                               } } );

                    img.texture->path = ktxPath.string();
                    imageData.emplace_back( std::move( img ) );
                }

                VkBufferCreateInfo stageBufferInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                                    .size = stageBufferSize,
                                                    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE };

                VmaAllocationCreateInfo stageAllocInfo{
                    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    .usage = VMA_MEMORY_USAGE_AUTO };

                m_vkCtx->device->createBuffer( container.stagingBuffer, stageBufferInfo, stageAllocInfo );
            } );

        task->SetDependency( task->dependency, generateKtx );
        taskManager->addTaskSetToPipe( generateKtx );

        m_textureLoadTasks.push_back( task );
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

    namespace fs = std::filesystem;
    fs::path p{ texturePath };
    std::string filename = p.stem().string() + ".ktx2";

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load( texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

    if ( !pixels )
    {
        throw std::runtime_error( "failed to load texture vulkanImage!" );
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

    m_vkCtx->device->createImage(
        texture->vulkanImage.vkImage, imageInfo, imageAllocInfo, texture->vulkanImage.vmaAllocation, textureName );

    m_vkCtx->device->createImageView( texture->vulkanImage.vkImageView,
                                      texture->vulkanImage.vkImage,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_ASPECT_COLOR_BIT );

    // transfer barrier
    VkImageMemoryBarrier2 transferLayoutBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                                 .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                                                 .srcAccessMask = VK_ACCESS_2_NONE,
                                                 .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                                                 .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                 .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                 .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                 .image = texture->vulkanImage.vkImage,
                                                 .subresourceRange = {
                                                     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                     .baseMipLevel = 0,
                                                     .levelCount = 1,
                                                     .baseArrayLayer = 0,
                                                     .layerCount = 1,
                                                 } };

    m_vkCtx->device->transitionImageLayout( texture->vulkanImage.vkImage, transferLayoutBarrier );

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
                                            .image = texture->vulkanImage.vkImage,
                                            .subresourceRange = {
                                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                .baseMipLevel = 0,
                                                .levelCount = 1,
                                                .baseArrayLayer = 0,
                                                .layerCount = 1,
                                            } };

    m_vkCtx->device->copyBufferToImage( stageBuffer.vkBuffer,
                                        texture->vulkanImage.vkImage,
                                        static_cast<uint32_t>( texWidth ),
                                        static_cast<uint32_t>( texHeight ),
                                        copyImageBarrier );

    m_vkCtx->device->destroyBuffer( stageBuffer );

    texture->path = texturePath;

    m_loadedTextures.emplace( texturePath, std::move( texture ) );
    return m_loadedTextures[texturePath].get();
}

auto core::AssetManager::getTextureSampler() -> VkSampler&
{
    return m_textureSampler;
}

auto core::AssetManager::initDescriptors() -> void
{
    createBindlessDescriptorSetLayout();
    allocateBindlessDescriptorSet();

    // materials
    createMaterialsBuffers( 1000 * sizeof( resources::Material ) );
    createMaterialsDescriptorSetLayout();
    allocateMaterialsDescriptorSet();
}

auto core::AssetManager::getTexture( const std::string& texturePath ) -> resources::Texture*
{
    std::filesystem::path p{ texturePath };

    if ( m_loadedTextures.contains( texturePath ) )
    {
        return m_loadedTextures[texturePath].get();
    }

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
    {
        return m_loadedMeshes[meshPath].get();
    }

    utilities::TaskManager* taskManager = core::MainRegistry::getInstance().getTaskManager();

    std::filesystem::path p{ meshPath };
    std::string filename{ p.stem().string() + "_collision.gltf" };
    std::filesystem::path collision = p.parent_path() / filename;

    if ( std::filesystem::exists( collision ) && !m_pendingMeshLoads.contains( collision.string() ) )
    {
        m_pendingMeshLoads.insert( collision.string() );

        utilities::DataTaskSet<MeshTaskData>* collisionMeshTask =
            taskManager->addTask( MeshTaskData{}, [this, collision]( MeshTaskData& data ) -> void {
                ZoneScopedN( "[TASK] collision mesh" );

                std::unique_ptr<TinyGltfLoader> tinyLoader = std::make_unique<TinyGltfLoader>( collision.string() );

                data.mesh = std::make_unique<resources::Mesh>();

                data.mesh->setPath( collision.string() );

                tinyLoader->processVertexData( data.mesh.get() );

                auto& indices = data.mesh->getIndices();
                auto& vertices = data.mesh->getVertices();

                data.verticesSize = sizeof( resources::Vertex ) * std::size( vertices );
                data.indicesSize = sizeof( uint32_t ) * std::size( indices );

                VkDeviceSize bufferSize = data.verticesSize + data.indicesSize;

                VkBufferCreateInfo stageBufferInfo{};
                stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                stageBufferInfo.size = bufferSize;
                stageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                stageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo stageAllocInfo{};
                stageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

                m_vkCtx->device->createBuffer( data.stageBuffer, stageBufferInfo, stageAllocInfo );

                VkBufferCreateInfo vertexBufferCreateInfo{};
                vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                vertexBufferCreateInfo.size = data.verticesSize;
                vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

                VkBufferCreateInfo indicesBufferCreateInfo{};
                indicesBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                indicesBufferCreateInfo.size = data.indicesSize;
                indicesBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

                VmaAllocationCreateInfo vmaAllocInfo{};
                vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
                vmaAllocInfo.flags =
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

                m_vkCtx->device->createBuffer( data.mesh->getVertexBufferObject(),
                                               vertexBufferCreateInfo,
                                               vmaAllocInfo,
                                               collision.filename().string() );

                m_vkCtx->device->createBuffer( data.mesh->getIndicesBufferObject(),
                                               indicesBufferCreateInfo,
                                               vmaAllocInfo,
                                               collision.filename().string() );

                // buffer
                // .....indices..../....vertices...

                m_vkCtx->device->copyMemoryToAllocation(
                    indices.data(), data.stageBuffer.vmaAllocation, 0, data.indicesSize );

                m_vkCtx->device->copyMemoryToAllocation(
                    vertices.data(), data.stageBuffer.vmaAllocation, data.indicesSize, data.verticesSize );
            } );

        taskManager->addTaskSetToPipe( collisionMeshTask );
        m_meshLoadTasks.emplace_back( collisionMeshTask );
    }

    if ( m_pendingMeshLoads.contains( meshPath ) )
        return nullptr;

    m_pendingMeshLoads.insert( meshPath );

    utilities::DataTaskSet<MeshTaskData>* meshAndMaterialsTask =
        taskManager->addTask( MeshTaskData{}, [this, meshPath]( MeshTaskData& data ) -> void {
            ZoneScopedN( "[TASK] mesh + materials" );
            std::unique_ptr<TinyGltfLoader> tinyLoader = std::make_unique<TinyGltfLoader>( meshPath );
            data.mesh = std::make_unique<resources::Mesh>();

            data.mesh->setPath( meshPath );

            tinyLoader->processVertexData( data.mesh.get() );

            // this could be redundant
            data.textureBatch.pMesh = data.mesh.get();
            data.textureBatch.parsedMaterialData = std::move( tinyLoader->parseTextureData( data.mesh.get() ) );

            auto& indices = data.mesh->getIndices();
            auto& vertices = data.mesh->getVertices();

            data.verticesSize = sizeof( resources::Vertex ) * std::size( vertices );
            data.indicesSize = sizeof( uint32_t ) * std::size( indices );

            VkDeviceSize bufferSize = data.verticesSize + data.indicesSize;

            VkBufferCreateInfo stageBufferInfo{};
            stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stageBufferInfo.size = bufferSize;
            stageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo stageAllocInfo{};
            stageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            m_vkCtx->device->createBuffer( data.stageBuffer, stageBufferInfo, stageAllocInfo );

            VkBufferCreateInfo vertexBufferCreateInfo{};
            vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            vertexBufferCreateInfo.size = data.verticesSize;
            vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

            VkBufferCreateInfo indicesBufferCreateInfo{};
            indicesBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            indicesBufferCreateInfo.size = data.indicesSize;
            indicesBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

            VmaAllocationCreateInfo vmaAllocInfo{};
            vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            vmaAllocInfo.flags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            std::filesystem::path meshP{ meshPath };

            m_vkCtx->device->createBuffer(
                data.mesh->getVertexBufferObject(), vertexBufferCreateInfo, vmaAllocInfo, meshP.filename().string() );

            m_vkCtx->device->createBuffer(
                data.mesh->getIndicesBufferObject(), indicesBufferCreateInfo, vmaAllocInfo, meshP.filename().string() );

            // buffer
            // .....indices..../....vertices...

            m_vkCtx->device->copyMemoryToAllocation(
                indices.data(), data.stageBuffer.vmaAllocation, 0, data.indicesSize );

            m_vkCtx->device->copyMemoryToAllocation(
                vertices.data(), data.stageBuffer.vmaAllocation, data.indicesSize, data.verticesSize );
        } );

    taskManager->addTaskSetToPipe( meshAndMaterialsTask );

    m_meshLoadTasks.emplace_back( meshAndMaterialsTask );

    return nullptr;
}

auto core::AssetManager::loadFont( const std::string_view path ) -> void
{
    KASSERT( std::filesystem::exists( path ) && "File does not exist" );
    ZoneScopedN( "AssetManager::loadFont" );

    m_fontLoader.generateAtlas( path );

    std::filesystem::path p{ path };
    std::string fontName = p.stem().string();
    std::unique_ptr<resources::Font> font =
        std::make_unique<resources::Font>( fontName,
                                           p.parent_path().string() + "/" + fontName + ".json",
                                           p.parent_path().string() + "/" + fontName + ".png" );
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

auto core::AssetManager::createIndexBuffer( resources::Mesh* pMesh, graphics::VulkanBuffer stageBuffer ) -> void
{
    std::vector<uint32_t>& indices = pMesh->getIndices();

    VmaAllocationInfo allocInfo{ m_vkCtx->device->getAllocInfo( stageBuffer.vmaAllocation ) };
    m_vkCtx->device->copyMemoryToAllocation( indices.data(), stageBuffer.vmaAllocation, 0, allocInfo.size );

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = allocInfo.size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    std::filesystem::path meshPath = pMesh->getPath();
    std::string name = meshPath.stem().string() + "_indicesBuff";

    m_vkCtx->device->createBuffer( pMesh->getIndicesBufferObject(), bufferInfo, vmaAllocInfo, name );
}

auto core::AssetManager::createVertexBuffer( resources::Mesh* pMesh, graphics::VulkanBuffer stageBuffer ) -> void
{
    std::vector<resources::Vertex>& vertices = pMesh->getVertices();

    VmaAllocationInfo allocInfo{ m_vkCtx->device->getAllocInfo( stageBuffer.vmaAllocation ) };
    m_vkCtx->device->copyMemoryToAllocation( vertices.data(), stageBuffer.vmaAllocation, 0, allocInfo.size );

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = allocInfo.size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    std::filesystem::path meshPath = pMesh->getPath();
    std::string name = meshPath.stem().string() + "_verticesBuff";

    m_vkCtx->device->createBuffer( pMesh->getVertexBufferObject(), bufferInfo, vmaAllocInfo, name );
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
    if ( !m_textureSampler )
    {
        throw std::runtime_error( "sampler is not initialized" );
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites{};
    std::vector<VkDescriptorImageInfo> imageInfo{};

    for ( resources::Texture* tex : textures )
    {
        imageInfo.push_back( VkDescriptorImageInfo{
            .sampler = m_textureSampler,
            .imageView = tex->vulkanImage.vkImageView,
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

        tex->textureIndex = m_bindlessTexturesIndex;

        ++m_bindlessTexturesIndex;
    }

    m_vkCtx->device->updateDescriptorSet( descriptorWrites );
}

auto core::AssetManager::updateBindlessTextures( resources::Texture* texture ) -> void
{
    if ( !texture->vulkanImage.vkImageView )
    {
        KERROR( "VkImageView not initialized" );
        return;
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->vulkanImage.vkImageView;
    imageInfo.sampler = m_textureSampler;

    VkWriteDescriptorSet bindlessDescriptor{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                             .dstSet = m_bindlessTexturesDescriptor.set,
                                             .dstBinding = 0,
                                             .dstArrayElement = m_bindlessTexturesIndex,
                                             .descriptorCount = 1,
                                             .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                             .pImageInfo = &imageInfo };

    texture->textureIndex = m_bindlessTexturesIndex;
    ++m_bindlessTexturesIndex;

    m_vkCtx->device->updateDescriptorSet( { bindlessDescriptor } );
}

auto core::AssetManager::createMaterialsDescriptorSet() -> void
{
    uint32_t descriptorCount{ 1000u };

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

    VkWriteDescriptorSet bindlessDescriptor{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                             .dstSet = m_materialsDescriptor.set,
                                             .dstBinding = 0,
                                             .dstArrayElement = 0,
                                             .descriptorCount = 1,
                                             .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                             .pBufferInfo = &buffInfo };

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

auto core::AssetManager::processTextureTasks() -> void
{
    if ( !m_textureLoadTasks.empty() )
    {
        ZoneScopedN( "cmd recording" );
        for ( auto it = m_textureLoadTasks.begin(); it != m_textureLoadTasks.end(); )
        {
            enki::ITaskSet* taskSet = *it;

            // search for the current task set we're checking to see if it is already present in the task sync vector
            const bool isInSync = std::ranges::find_if( m_textureTaskSync, [taskSet]( TaskSync& t ) {
                                      return t.task == taskSet;
                                  } ) != m_textureTaskSync.end();

            // continue since the taskset has its data already processed and waiting on a semaphore
            if ( isInSync )
            {
                ++it;
                continue;
            }

            if ( !taskSet->GetIsComplete() )
            {
                ++it;
                continue;
            }

            // cast to get access to the data produced by the task
            auto* dataTaskSetPtr = static_cast<utilities::DataTaskSet<TextureTaskData>*>( taskSet );

            TextureTaskData& data = dataTaskSetPtr->container;

            VkDependencyInfo before{};
            before.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            before.imageMemoryBarrierCount = data.beforeBarriers.size();
            before.pImageMemoryBarriers = data.beforeBarriers.data();

            VkCommandPool commandPool = m_vkCtx->device->getTransferCommandPool();
            VkCommandBuffer commandBuffer = m_vkCtx->device->beginSingleTimeCommands( commandPool );

            vkCmdPipelineBarrier2( commandBuffer, &before );

            VkDeviceSize currentOffset{ 0 };

            for ( const ImageData& img : data.imageData )
            {
                if ( !img.data )
                {
                    continue;
                }

                m_vkCtx->device->copyMemoryToAllocation(
                    img.data, data.stagingBuffer.vmaAllocation, currentOffset, img.size );

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

                vkCmdCopyBufferToImage( commandBuffer,
                                        data.stagingBuffer.vkBuffer,
                                        img.texture->vulkanImage.vkImage,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        1,
                                        &region );
            }

            VkDependencyInfo after{};
            after.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            after.imageMemoryBarrierCount = data.afterBarriers.size();
            after.pImageMemoryBarriers = data.afterBarriers.data();

            vkCmdPipelineBarrier2( commandBuffer, &after );

            VkQueue transferQueue = m_vkCtx->device->getTransferQueue().handle;

            m_vkCtx->device->endSingleTimeCommands(
                commandBuffer, transferQueue, commandPool, m_textureTimelineSemaphore, m_textureTimelineSignal );

            // now this taskSync is waiting on the semaphore to be signaled by the queue submition
            m_textureTaskSync.emplace_back( taskSet, m_textureTimelineSignal );
        }
    }
}

auto core::AssetManager::updateTextureTaskSync() -> void
{
    utilities::TaskManager* taskManager = core::MainRegistry::getInstance().getTaskManager();

    uint64_t value{ 0u };
    m_vkCtx->device->getSemaphoreCounterValue( m_textureTimelineSemaphore, value );

    std::erase_if( m_textureTaskSync, [&]( const TaskSync& taskSet ) {
        ZoneScopedN( "erase taskSync" );

        // if not signaled ( GPU hasn't finished upload )
        if ( taskSet.semaphoreWaitValue > value )
        {
            return false;
        }

        auto* dataTaskSetPtr = static_cast<utilities::DataTaskSet<TextureTaskData>*>( taskSet.task );

        TextureTaskData& data = dataTaskSetPtr->container;

        for ( ImageData& img : data.imageData )
        {
            if ( !img.data )
            {
                continue;
            }

            updateBindlessTextures( img.texture.get() );

            m_ktxTextureManager->destroyTexture( img.texture.get() );
            m_pendingTextureLoads.erase( img.texture->path );
            m_loadedTextures.emplace( img.texture->path, std::move( img.texture ) );
        }

        resolveMaterials( data.materialsBatch );
        updateMaterialsBuffer();

        m_vkCtx->device->destroyBuffer( data.stagingBuffer );

        std::erase( m_textureLoadTasks, taskSet.task );
        taskManager->eraseTask( taskSet.task, utilities::TaskType::Data );

        return true;
    } );
}

auto core::AssetManager::processMeshTasks() -> void
{
    if ( !m_meshLoadTasks.empty() )
    {
        ZoneScopedN( "mesh cmd recording" );
        for ( auto it = m_meshLoadTasks.begin(); it != m_meshLoadTasks.end(); )
        {
            enki::ITaskSet* taskSet = *it;

            // search for the current task set we're checking to see if it is already present in the task sync vector
            const bool isInSync = std::ranges::find_if( m_meshTaskSync, [taskSet]( TaskSync& t ) {
                                      return t.task == taskSet;
                                  } ) != m_meshTaskSync.end();

            // continue since the taskset has its data already processed and waiting on a semaphore
            if ( isInSync )
            {
                ++it;
                continue;
            }

            if ( !taskSet->GetIsComplete() )
            {
                ++it;
                continue;
            }

            // cast to get access to the data produced by the task
            auto* dataTaskSetPtr = static_cast<utilities::DataTaskSet<MeshTaskData>*>( taskSet );

            MeshTaskData& data = dataTaskSetPtr->container;

            // copy the data from the staging buffer to the mesh buffers
            VkCommandBuffer cmdBuffer =
                m_vkCtx->device->beginSingleTimeCommands( m_vkCtx->device->getTransferCommandPool() );

            VkBufferCopy indicesCopyRegion{};
            indicesCopyRegion.size = data.indicesSize;
            indicesCopyRegion.srcOffset = 0;
            indicesCopyRegion.dstOffset = 0;

            vkCmdCopyBuffer( cmdBuffer,
                             data.stageBuffer.vkBuffer,
                             data.mesh->getIndicesBufferObject().vkBuffer,
                             1,
                             &indicesCopyRegion );

            VkBufferCopy verticesCopyRegion{};
            verticesCopyRegion.size = data.verticesSize;
            verticesCopyRegion.srcOffset = data.indicesSize;
            verticesCopyRegion.dstOffset = 0;

            vkCmdCopyBuffer( cmdBuffer,
                             data.stageBuffer.vkBuffer,
                             data.mesh->getVertexBufferObject().vkBuffer,
                             1,
                             &verticesCopyRegion );

            m_vkCtx->device->endSingleTimeCommands( cmdBuffer,
                                                    m_vkCtx->device->getTransferQueue().handle,
                                                    m_vkCtx->device->getTransferCommandPool(),
                                                    m_meshTimelineSemaphore,
                                                    m_meshTimelineSignal );

            // now this taskSync is waiting on the semaphore to be signaled by the queue submition
            m_meshTaskSync.emplace_back( taskSet, m_meshTimelineSignal );
        }
    }
}

auto core::AssetManager::updateMeshTaskSync() -> void
{
    utilities::TaskManager* taskManager = core::MainRegistry::getInstance().getTaskManager();

    uint64_t value{ 0u };
    m_vkCtx->device->getSemaphoreCounterValue( m_meshTimelineSemaphore, value );

    std::erase_if( m_meshTaskSync, [&]( const TaskSync& taskSet ) {
        ZoneScopedN( "erase meshTaskSync" );

        // if not signaled ( GPU hasn't finished upload )
        if ( taskSet.semaphoreWaitValue > value )
        {
            return false;
        }

        auto* dataTaskSetPtr = static_cast<utilities::DataTaskSet<MeshTaskData>*>( taskSet.task );

        MeshTaskData& data = dataTaskSetPtr->container;

        m_vkCtx->device->destroyBuffer( data.stageBuffer );

        std::string meshPath = data.mesh->getPath();
        data.mesh->setLoaded( true );
        m_loadedMeshes.emplace( meshPath, std::move( data.mesh ) );

        data.textureBatch.pMesh = m_loadedMeshes[meshPath].get();

        // collision meshes have no textures
        if ( !data.textureBatch.parsedMaterialData.empty() )
        {
            // this might not be needed as queue
            m_parsedMaterialsQueue.push( data.textureBatch );
            loadTextureData();
        }

        m_pendingMeshLoads.erase( meshPath );

        std::erase( m_meshLoadTasks, taskSet.task );
        taskManager->eraseTask( taskSet.task, utilities::TaskType::Data );

        return true;
    } );
}

auto core::AssetManager::onUpdate() -> void
{
    utilities::TaskManager* taskManager = core::MainRegistry::getInstance().getTaskManager();

    processMeshTasks();
    updateMeshTaskSync();

    processTextureTasks();
    updateTextureTaskSync();
}

auto core::AssetManager::enqueueMesh( resources::Mesh* mesh ) -> void
{
    m_queuedMeshes.push( mesh );
}

auto core::AssetManager::addUiTexture( const std::filesystem::path p ) -> void
{
    m_uiTexturePaths.push_back( p );
}

auto core::AssetManager::resolveMaterials( ParsedMaterials batch ) -> void
{
    ZoneScopedN( "resolveMaterials" );

    auto getKtxPath = []( const std::string& path ) -> std::filesystem::path {
        std::filesystem::path texPath = std::filesystem::path{ path };
        std::string filename = texPath.stem().string() + ".ktx2";
        return texPath.parent_path() / filename;
    };

    for ( auto& [submesh, textureMap] : batch.parsedMaterialData )
    {
        resources::Material material{};

        auto& submeshes = batch.pMesh->getSubmeshes();

        resources::Texture* diffuse = getTexture( getKtxPath( textureMap.at( Diffuse ) ).string() );
        if ( diffuse )
        {
            material.diffuseTextureIndex = diffuse->textureIndex;
        }

        resources::Texture* emissive = getTexture( getKtxPath( textureMap.at( Emissive ) ).string() );
        if ( emissive )
        {
            material.emissiveTextureIndex = emissive->textureIndex;
        }

        resources::Texture* normal = getTexture( getKtxPath( textureMap.at( Normal ) ).string() );
        if ( normal )
        {
            material.normalTextureIndex = normal->textureIndex;
        }

        resources::Material stale{};
        if ( material == stale )
        {
            continue;
        }

        auto it = std::ranges::find( m_materials, material );

        if ( it != m_materials.end() )
        {
            submeshes[submesh].materialIndex = std::distance( m_materials.begin(), it );
        }
        else
        {
            submeshes[submesh].materialIndex = m_materials.size();
            m_materials.push_back( material );
        }
    }
}
