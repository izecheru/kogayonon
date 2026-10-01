#include "core/asset_manager/ktx_texture_manager.hpp"
#include "utilities/utils/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include <stb_image.h>
#include "resources/texture.hpp"

core::KtxTextureManager::KtxTextureManager( graphics::VulkanDevice* device )
    : m_device{ device }
{
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
    auto result =
        ktxTexture2_CreateFromNamedFile( p.string().c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &texture->ktxImage.texture );

    if ( result != KTX_SUCCESS )
    {
        return result;
    }

    result = ktxTexture2_VkUploadEx( texture->ktxImage.texture,
                                     &m_vulkanDeviceInfo,
                                     &texture->ktxImage.vulkanTexture,
                                     VK_IMAGE_TILING_OPTIMAL,
                                     VK_IMAGE_USAGE_SAMPLED_BIT,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

    if ( result != KTX_SUCCESS )
    {
        ktxTexture2_Destroy( texture->ktxImage.texture );
        return result;
    }

    texture->ktxImage.uploaded = true;

    return KTX_SUCCESS;
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

auto core::KtxTextureManager::convertToKtx( const std::filesystem::path p, KtxTextureData data ) -> KTX_error_code
{
    KINFO( "Converting texture {} to KTX format", p.stem().string() );

    if ( !data.pixels )
    {
        return KTX_FILE_OPEN_FAILED;
    }

    ktxTextureCreateInfo createInfo{};
    createInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    createInfo.baseWidth = static_cast<ktx_uint32_t>( data.w );
    createInfo.baseHeight = static_cast<ktx_uint32_t>( data.h );
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;

    ktxTexture2* texture = nullptr;

    KTX_error_code result = ktxTexture2_Create( &createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture );

    if ( result != KTX_SUCCESS )
    {
        ktxTexture2_Destroy( texture );
        return result;
    }

    const std::string filename = p.stem().string() + ".ktx2";
    const std::filesystem::path ktxPath =
        std::filesystem::current_path() / "engine_resources" / "textures" / "ktx" / filename;

    ktxTexture_SetImageFromMemory( ktxTexture( texture ), 0, 0, 0, data.pixels, data.w * data.h * data.c );

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

    if ( texture->ktxImage.uploaded )
    {
        ktxVulkanTexture_Destruct( &texture->ktxImage.vulkanTexture, m_device->getLogicalDevice(), nullptr );
    }

    if ( texture->ktxImage.texture )
    {
        ktxTexture2_Destroy( texture->ktxImage.texture );
    }
}