#pragma once
#include "core/asset_manager/assimp_loader.hpp"
#include "core/asset_manager/cgltf_loader.hpp"
#include "core/asset_manager/font_loader.hpp"
#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_descriptor.hpp"
#include "precompiled/pch.hpp"
#include "resources/material.hpp"
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#define MAX_TEXTURE_SUPPORT 1000

namespace graphics
{
struct VulkanContext;
}

namespace resources
{
class Texture;
class Mesh;
class Font;
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
	AssetManager();
	~AssetManager();

	/**
	 * @brief Set the VulkanContext* member variable
	 * @param ctx
	 */
	auto setContext( graphics::VulkanContext* ctx ) -> void;

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

	// Fonts
	auto loadFont( const std::string_view path ) -> void;

	// TODO(kogayonon) change this
	auto setDescriptorPool( VkDescriptorPool pool ) -> void;
	auto initDescriptors() -> void;

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

	auto getMeshes() -> std::unordered_map<std::string, std::shared_ptr<resources::Mesh>>&;
	auto getTextures() -> std::unordered_map<std::string, std::shared_ptr<resources::Texture>>&;

  private:
	/**
	 * @brief Uses the vertices VulkanBuffer of a mesh to fill it up with Vertex data from model file
	 * @param pMesh Pointer to the mesh
	 */
	auto createVertexBuffer( resources::Mesh* pMesh ) -> void;

	/**
	 * @brief Uses the indices VulkanBuffer of a mesh to fill it up with uint32_t indices data from model file
	 * @param pMesh Pointer to the mesh
	 */
	auto createIndexBuffer( resources::Mesh* pMesh ) -> void;

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
	auto createMaterialsBuffers() -> void;

	/**
	 * @brief When we add a new material, we need to update the buffer so we have those changes on the gpu too
	 */
	auto updateMaterialsBuffer() -> void;

  private:
	// copy is not allowed
	AssetManager( const AssetManager& ) = delete;
	AssetManager& operator=( const AssetManager& ) = delete;
	AssetManager( AssetManager&& ) = delete;
	AssetManager& operator=( AssetManager&& ) = delete;

  private:
	VkDescriptorPool m_pDescriptorPool{ nullptr };

	bool m_layoutInit{ false };

	graphics::VulkanDescriptor m_bindlessTexturesDescriptor;
	graphics::VulkanDescriptor m_materialsDescriptor;

	graphics::VulkanBuffer m_materialsBuffer;
	std::vector<resources::Material> m_materials;

	std::unordered_map<std::string, std::shared_ptr<resources::Texture>> m_loadedTextures;
	std::unordered_map<std::string, std::shared_ptr<resources::Mesh>> m_loadedMeshes;
	std::unordered_map<std::string, std::shared_ptr<resources::Font>> m_loadedFonts;

	graphics::VulkanContext* m_vkCtx;

	VkSampler m_textureSampler;

	CgltfLoader m_cgltfLoader;
	AssimpLoader m_assimpLoader;
	FontLoader m_fontLoader;

	// used for assigning the values to material indices in the mesh
	uint32_t m_bindlessTexturesIndex;
	uint32_t m_materialIndex;
	uint32_t m_samplerIndex;
};
} // namespace core