#include "core/asset_manager/ktx_texture_manager.hpp"
#include "utilities/utils/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include <vma/vk_mem_alloc.h>
#include <stb_image.h>
#include "resources/texture.hpp"

core::KtxTextureManager::KtxTextureManager( graphics::VulkanDevice* device )
    : m_device{ device }
    , m_subAllocatorCallbacks{}

{
    // g_Allocator = device->getAllocator();

    ktxVulkanDeviceInfo_Construct( &m_vulkanDeviceInfo,
                                   device->getPhysicalDevice(),
                                   device->getLogicalDevice(),
                                   device->getGraphicsQueue().handle,
                                   device->getCommandPool(),
                                   nullptr );
}

core::KtxTextureManager::~KtxTextureManager()
{
    ktxVulkanDeviceInfo_Destruct( &m_vulkanDeviceInfo );
}

auto core::KtxTextureManager::loadTexture( const std::filesystem::path p, resources::Texture* texture )
    -> KTX_error_code
{
    KTX_error_code result = ktxTexture2_CreateFromNamedFile(
        p.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture->ktxImage.texture );

    return result;
}

auto core::KtxTextureManager::saveTexture( const std::filesystem::path p, resources::Texture* texture )
    -> KTX_error_code
{
    if ( !texture || !texture->ktxImage.texture )
    {
        return KTX_INVALID_VALUE;
    }

    return ktxTexture2_WriteToNamedFile( texture->ktxImage.texture, p.string().c_str() );
}

auto core::KtxTextureManager::saveToKtx( const std::filesystem::path p ) -> KTX_error_code
{
    int w, c, h;
    uint8_t* pixels = stbi_load( p.string().c_str(), &w, &h, &c, STBI_rgb_alpha );

    if ( !pixels )
    {
        return KTX_FILE_OPEN_FAILED;
    }

    ktxTextureCreateInfo createInfo{};
    createInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    createInfo.baseWidth = static_cast<ktx_uint32_t>( w );
    createInfo.baseHeight = static_cast<ktx_uint32_t>( h );
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;

    ktxTexture2* texture{ nullptr };

    KTX_error_code result = ktxTexture2_Create( &createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture );

    if ( result != KTX_SUCCESS )
    {
        ktxTexture2_Destroy( texture );
        return result;
    }

    const std::string filename = p.stem().string() + ".ktx2";
    const std::filesystem::path ktxPath =
        std::filesystem::current_path() / "engine_resources" / "textures" / "ktx" / filename;

    ktxTexture_SetImageFromMemory( ktxTexture( texture ), 0, 0, 0, pixels, w * h * 4 );

    result = ktxTexture2_WriteToNamedFile( texture, ktxPath.string().c_str() );
    ktxTexture2_Destroy( texture );
    return result;
}

auto core::KtxTextureManager::destroyTexture( resources::Texture* texture ) -> void
{
    if ( !texture )
    {
        return;
    }

    if ( texture->ktxImage.texture )
    {
        ktxTexture2_Destroy( texture->ktxImage.texture );
    }
}

auto core::KtxTextureManager::getTextureData( resources::Texture* texture ) -> uint8_t*
{
    return ktxTexture_GetData( ktxTexture( texture->ktxImage.texture ) );
}

auto core::KtxTextureManager::getDataSize( resources::Texture* texture ) -> uint32_t
{
    return ktxTexture_GetDataSize( ktxTexture( texture->ktxImage.texture ) );
}
