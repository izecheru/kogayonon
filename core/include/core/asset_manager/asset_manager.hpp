#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include "core/asset_manager/font_loader.hpp"
#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_descriptor.hpp"
#include "core/asset_manager/tinygltf_loader.hpp"
#include "core/asset_manager/threadsafe_resource_manager.hpp"

namespace enki
{
struct ITaskSet;
}

namespace resources
{
struct Texture;
class Mesh;
class Font;
struct Material;
} // namespace resources

namespace graphics
{
struct VulkanContext;
}

namespace core
{
class KtxTextureManager;

struct ParsedMaterials
{
    /**
     * @brief Owner of those materials
     */
    resources::Mesh* pMesh{ nullptr };

    std::unordered_map<uint32_t, std::map<TextureType, std::string>> parsedMaterialData;
};

struct ImageData
{
    /**
     * @brief Texture related data
     */
    int w;
    int h;
    int c;
    uint8_t* data;

    /**
     * @brief Actual image byte size
     */
    VkDeviceSize size;

    std::unique_ptr<resources::Texture> texture;
};

struct MeshTaskData
{
    std::unique_ptr<resources::Mesh> mesh;
    graphics::VulkanBuffer stageBuffer;

    VkDeviceSize indicesSize{ 0u };
    VkDeviceSize verticesSize{ 0u };

    ParsedMaterials textureBatch;
};

/**
 * @brief Everything that a load texture task produces
 */
struct TextureTaskData
{
    std::vector<ImageData> imageData;

    /**
     * @brief Memory barriers for before and after vulkan copy commands
     */
    std::vector<VkImageMemoryBarrier2> beforeBarriers;
    std::vector<VkImageMemoryBarrier2> afterBarriers;

    /**
     * @brief Buffer size needed for the texture data transfer, we use only one staging buffer
     */
    VkDeviceSize stageBufferSize{ 0u };

    /**
     * @brief This is used to resolve materials, once textures are all loaded we search in the
     * material data provided by the tinygltf loader and assing the current bindless index to that
     * specific texture. We go through emissive, normal and diffuse textures to build an entire material
     */
    ParsedMaterials materialsBatch;

    graphics::VulkanBuffer stagingBuffer;
};

struct TaskSync
{
    /**
     * @brief Task pointer so we retrieve the data produced by the task thread
     */
    enki::ITaskSet* task{ nullptr };

    /**
     * @brief This is so we know the data produced by the task thread can be safely used in
     * vulkan command recording since atm we do that on the main thread
     */
    uint64_t semaphoreWaitValue{ 0u };
};

} // namespace core

#define MAX_TEXTURE_SUPPORT 1000

namespace core
{
class AssetManager
{
  public:
    explicit AssetManager( graphics::VulkanContext* vkCtx );
    ~AssetManager();

    auto recreate() -> void;

    auto init() -> void;
    auto destroyResources() -> void;

    auto processTextureTasks() -> void;
    auto updateTextureTaskSync() -> void;

    auto processMeshTasks() -> void;
    auto updateMeshTaskSync() -> void;

    /**
     * @brief Get the texture sampler of the AssetManager, this is for convenience
     * @return
     */
    auto getTextureSampler() -> VkSampler&;

    /**
     * @brief Initialize sampler
     */
    auto initSampler() -> void;

    /**
     * @brief Get the texture using texture absolute path as key in the map
     * @param texturePath
     * @return
     */
    auto getTexture( const std::string& texturePath ) -> resources::Texture*;

    /**
     * @brief Loads texture if it is not already loaded
     * @param textureName Name of the texture
     * @param texturePath Absolute path to texture
     * @return
     */
    auto loadTexture( const std::string& textureName, const std::string& texturePath ) -> resources::Texture*;
    auto loadTextureData() -> void;

    /**
     * @brief Get the mesh using absolute path as key
     * @param path
     * @return
     */
    auto getMesh( const std::string& path ) -> resources::Mesh*;

    /**
     * @brief Load mesh if it is not already loaded
     * @param meshName Name of the mesh
     * @param meshPath Absolute path to the mesh
     * @return
     */
    auto loadMesh( const std::string& meshName, const std::string& meshPath ) -> resources::Mesh*;

    auto uploadMeshData() -> void;

    /**
     * @brief Enqueue mesh for vertices and indices buffers creation
     * @param mesh
     * @return
     */
    auto enqueueMesh( resources::Mesh* mesh ) -> void;

    auto onUpdate() -> void;

    auto loadFont( const std::string_view path ) -> void;

    auto initDescriptors() -> void;

    /**
     * @brief We need this layout for the VkPipelineCreateInfo structure
     * @return
     */
    auto getBindlessTexturesDescriptorLayout() -> VkDescriptorSetLayout&;

    /**
     * @brief We get the descriptor set to bind it when rendering
     * @return
     */
    auto getBindlessDescriptorSet() -> VkDescriptorSet&;

    /**
     * @brief We need this layout for the VkPipelineCreateInfo structure
     * @return
     */
    auto getMaterialsDescriptorLayout() -> VkDescriptorSetLayout&;

    /**
     * @brief We get the descriptor set to bind it when rendering
     * @return
     */
    auto getMaterialsDescriptorSet() -> VkDescriptorSet&;

    auto getMeshes() -> std::unordered_map<std::string, std::unique_ptr<resources::Mesh>>&;
    auto getTextures() -> std::unordered_map<std::string, std::unique_ptr<resources::Texture>>&;

    /**
     * @brief Populates a vector of texture paths and protects them from runtime destruction
     * @param p
     * @return
     */
    auto addUiTexture( const std::filesystem::path p ) -> void;

    template <typename T>
    auto hasResource( const std::string& resourceKey ) -> bool
    {
        if constexpr ( std::is_same_v<resources::Mesh, T> )
        {
            return m_loadedMeshes.find( resourceKey ) != m_loadedMeshes.end();
        }
        else if constexpr ( std::is_same_v<resources::Texture, T> )
        {
            return m_loadedTextures.find( resourceKey ) != m_loadedTextures.end();
        }
    }

  private:
    /**
     * @brief Uses the vertices VulkanBuffer of a mesh to fill it up with Vertex data from model file
     * @param pMesh Pointer to the mesh
     */
    auto createVertexBuffer( resources::Mesh* pMesh, graphics::VulkanBuffer stageBuffer ) -> void;

    /**
     * @brief Uses the indices VulkanBuffer of a mesh to fill it up with uint32_t indices data from model file
     * @param pMesh Pointer to the mesh
     */
    auto createIndexBuffer( resources::Mesh* pMesh, graphics::VulkanBuffer stageBuffer ) -> void;

    /**
     * @brief Create descriptor layout for the bindless texture array
     */
    auto createBindlessDescriptorSetLayout() -> void;

    /**
     * @brief Allocate descriptor sets in the pool
     */
    auto allocateBindlessDescriptorSet() -> void;

    /**
     * @brief Update the bindless texture descriptor set to include a new texture in the array
     * @param pTexture Texture pointer
     */
    auto updateBindlessTextures( resources::Texture* pTexture ) -> void;
    auto updateBindlessTextures( const std::vector<resources::Texture*>& textures ) -> void;

    /**
     * @brief Create the descriptor layout for the material array ssbo
     */
    auto createMaterialsDescriptorSetLayout() -> void;

    /**
     * @brief Allocate descriptor sets for the material array
     */
    auto allocateMaterialsDescriptorSet() -> void;

    /**
     * @brief Create the Descriptor Set for the material array
     */
    auto createMaterialsDescriptorSet() -> void;

    /**
     * @brief Create the materials ssbo for the first time
     */
    auto createMaterialsBuffers( VkDeviceSize size ) -> void;

    /**
     * @brief When we add a new material, we need to update the buffer so we have those changes on the gpu too
     */
    auto updateMaterialsBuffer() -> void;

    auto resolveMaterials( ParsedMaterials batch ) -> void;

    auto shutdown() -> void;

  private:
    AssetManager( const AssetManager& ) = delete;
    AssetManager& operator=( const AssetManager& ) = delete;
    AssetManager( AssetManager&& ) = delete;
    AssetManager& operator=( AssetManager&& ) = delete;

  private:
    VkDescriptorPool m_pDescriptorPool{ nullptr };
    graphics::VulkanDescriptor m_bindlessTexturesDescriptor;
    graphics::VulkanDescriptor m_materialsDescriptor;

    graphics::FrameInFlightVulkanBuffer m_materialsBuffer;
    std::vector<resources::Material> m_materials;

    std::unordered_map<std::string, std::unique_ptr<resources::Texture>> m_loadedTextures;
    std::unordered_map<std::string, std::unique_ptr<resources::Mesh>> m_loadedMeshes;
    std::queue<resources::Mesh*> m_queuedMeshes;
    std::unordered_map<std::string, std::unique_ptr<resources::Font>> m_loadedFonts;
    std::queue<ParsedMaterials> m_parsedMaterialsQueue;

    graphics::VulkanContext* m_vkCtx;
    VkSampler m_textureSampler;

    FontLoader m_fontLoader;
    std::unique_ptr<KtxTextureManager> m_ktxTextureManager;

    std::vector<graphics::VulkanBuffer> m_stagingBuffers;
    graphics::VulkanBuffer m_stagingBuffer;

    std::vector<std::filesystem::path> m_uiTexturePaths;

    // used for assigning the values to material indices in the mesh
    uint32_t m_bindlessTexturesIndex;

    uint32_t m_samplerIndex; // currently unused

    // mutexes
    std::mutex m_materialMutex;
    std::mutex m_meshMutex;
    std::mutex m_materialBufferMutex;

    // Task related member variables
    std::atomic<bool> m_readyForUpdate{ false };
    // ThreadsafeResourceManager m_threadSafeResourceManager;
    VkSemaphore m_textureTimelineSemaphore;
    uint64_t m_textureTimelineSignal{ 0u };
    VkSemaphore m_meshTimelineSemaphore;
    uint64_t m_meshTimelineSignal{ 0u };

    std::vector<enki::ITaskSet*> m_textureLoadTasks;
    std::vector<enki::ITaskSet*> m_meshLoadTasks;
    std::vector<TaskSync> m_textureTaskSync;
    std::vector<TaskSync> m_meshTaskSync;
    std::unordered_set<std::string> m_pendingTextureLoads;
    std::unordered_set<std::string> m_pendingMeshLoads;

    std::atomic<bool> m_acceptingLoads{ false };
    std::atomic<bool> m_shutdown{ false };
};
} // namespace core
