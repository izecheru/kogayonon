#pragma once
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include "core/asset_manager/assimp_loader.hpp"
#include "core/asset_manager/cgltf_loader.hpp"
#include "graphics/vulkan_buffer.hpp"
#include "precompiled/pch.hpp"
#include "resources/material.hpp"

namespace graphics
{
struct VulkanContext;
}

namespace resources
{
class Texture;
class Mesh;
} // namespace resources

struct cgltf_primitive;
struct cgltf_accessor;
struct cgltf_material;

struct AssimpMaterial
{
  std::string texturePath;
  int textureType;
};

namespace core
{
class AssetManager
{
public:
  /**
   * @brief Get the AssetManager instance
   * @return
   */
  static auto getInstance() -> AssetManager&
  {
    static auto instance = AssetManager{};
    return instance;
  }

  /**
   * @brief Set the VulkanContext* member variable
   * @param ctx
   */
  void setContext( graphics::VulkanContext* ctx );

  /**
   * @brief Get the texture sampler of the AssetManager, this is for convenience
   * @return
   */
  auto getTextureSampler() -> VkSampler&;

  /**
   * @brief Initialize sampler
   */
  void initSampler();

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

  /**
   * @brief Get the mesh using absolute path as key
   * @param path
   * @return
   */
  auto getMesh( const std::string& path ) -> std::optional<resources::Mesh*>;

  /**
   * @brief Load mesh if it is not already loaded
   * @param meshName Name of the mesh
   * @param meshPath Absolute path to the mesh
   * @return
   */
  auto loadMesh( const std::string& meshName, const std::string& meshPath ) -> resources::Mesh*;

  // TODO(kogayonon) change this
  void setDescriptorPool( VkDescriptorPool* pool );
  void initDescriptors();

  /**
   * @brief We need this layout for the VkPipelineCreateInfo structure
   * @return
   */
  auto getBindlessDescriptorLayout() -> VkDescriptorSetLayout&;

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

private:
  /**
   * @brief Uses the vertices VulkanBuffer of a mesh to fill it up with Vertex data from model file
   * @param pMesh Pointer to the mesh
   */
  void createVertexBuffer( resources::Mesh* pMesh );

  /**
   * @brief Uses the indices VulkanBuffer of a mesh to fill it up with uint32_t indices data from model file
   * @param pMesh Pointer to the mesh
   */
  void createIndexBuffer( resources::Mesh* pMesh );

  /**
   * @brief Create descriptor layout for the bindless texture array
   */
  void createBindlessDescriptorSetLayout();

  /**
   * @brief Allocate descriptor sets in the pool
   */
  void allocateBindlessDescriptorSet();

  /**
   * @brief Update the bindless texture descriptor set to include a new texture in the array
   * @param pTexture Texture pointer
   */
  void updateBindlessTextures( resources::Texture* pTexture );

  /**
   * @brief Create the descriptor layout for the material array ssbo
   */
  void createMaterialsDescriptorSetLayout();

  /**
   * @brief Allocate descriptor sets for the material array
   */
  void allocateMaterialsDescriptorSet();

  /**
   * @brief Create the Descriptor Set for the material array
   */
  void createMaterialsDescriptorSet();

  /**
   * @brief Create the materials ssbo for the first time
   */
  void createMaterialsBuffers();

  /**
   * @brief When we add a new material, we need to update the buffer so we have those changes on the gpu too
   */
  void updateMaterialsBuffer();

private:
  AssetManager() = default;
  ~AssetManager() = default;

  // copy is not allowed
  AssetManager( const AssetManager& ) = delete;
  AssetManager& operator=( const AssetManager& ) = delete;
  AssetManager( AssetManager&& ) = delete;
  AssetManager& operator=( AssetManager&& ) = delete;

private:
  VkDescriptorPool* m_pDescriptorPool{ nullptr };

  VkDescriptorSetLayout m_bindlessTexturesDescriptorSetLayout;
  bool m_layoutInit{ false };
  VkDescriptorSet m_bindlessTextureDescriptorSet;

  VkDescriptorSetLayout m_materialsDescriptorSetLayout;
  std::vector<VkDescriptorSet> m_materialsDescriptorSets;
  std::vector<graphics::VulkanBuffer> m_materialsBuffer;
  std::vector<resources::Material> m_materials;

  std::unordered_map<std::string, std::shared_ptr<resources::Texture>> m_loadedTextures;
  std::unordered_map<std::string, std::shared_ptr<resources::Mesh>> m_loadedMeshes;

  graphics::VulkanContext* m_pVkContext;

  VkSampler m_textureSampler;
  CgltfLoader m_cgltfLoader;
  AssimpLoader m_assimpLoader;

  // used for assigning the values to material indices in the mesh
  uint32_t m_bindlessTexturesIndex;
  uint32_t m_materialIndex;
};
} // namespace core