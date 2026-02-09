#pragma once
#include <filesystem>
#include <glm/glm.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include "resources/mesh.hpp"
#include "resources/texture.hpp"

namespace kogayonon_resources
{
struct Skeleton;
struct Joint;
} // namespace kogayonon_resources
// forward declarations of cgltf types
struct cgltf_primitive;
struct cgltf_accessor;
struct cgltf_material;
struct cgltf_skin;
struct cgltf_node;
struct cgltf_data;

namespace kogayonon_utilities
{
class AssetManager
{
public:
  inline static AssetManager& getInstance()
  {
    static AssetManager instance;
    return instance;
  }

  /**
   * @brief Loads a texture from /resources/textures/<textureName>, texture name must hold the extension too
   * @param textureName The name and extension of the texture eg. "default.png"
   * @return A weak_ptr to that loaded texture
   */
  auto addTexture( const std::string& textureName ) -> kogayonon_resources::Texture*;
  auto addTexture( const std::string& textureName, const std::string& texturePath ) -> kogayonon_resources::Texture*;
  auto addTextureWithoutParams( const std::string& textureName, const std::string& texturePath )
    -> std::weak_ptr<kogayonon_resources::Texture>;
  auto addTextureFromMemory( const std::string& textureName, const unsigned char* data )
    -> std::weak_ptr<kogayonon_resources::Texture>;

  /**
   * @brief Finds a texture by name, if no directory is provided it defaults to "resources/textures/"
   * @param textureName Name of the texture you are looking for
   * @param folder Folder we retrieve the texture from, defaults to "resources/textures/" if not specified as param
   * @return A weak ptr to the texture found
   */
  auto getTexture( const std::string& textureName, const std::string& folder = "resources/textures/" )
    -> std::weak_ptr<kogayonon_resources::Texture>;

  /**
   * @brief Deletes a texture from the loaded map, even though we index with texture name which is not actual
   * filename, we will loop through the map with an iterator it and look if the path == it->second->getPath() since we
   * store the path in the texture object
   * @param path Path of the texture file
   */
  void removeTexture( const std::string& path );

  auto getTextureById( uint32_t id ) -> std::weak_ptr<kogayonon_resources::Texture>;

  // meshes
  auto addMesh( const std::string& meshName, const std::string& meshPath ) -> kogayonon_resources::Mesh*;
  auto addMesh( const std::string& meshName ) -> kogayonon_resources::Mesh*;
  auto getMesh( const std::string& meshPath ) -> kogayonon_resources::Mesh*;

  /**
   * @brief Uploads each mesh data to the gpu and tells it how to interpret every buffer
   * @param meshes A vector of meshes that will need to be prepared for rendering
   */
  void uploadMeshGeometry( kogayonon_resources::Mesh* mesh ) const;

private:
  AssetManager();
  ~AssetManager();

  void destroy();
  void init();

  // copy is not allowed
  AssetManager( const AssetManager& ) = delete;
  AssetManager& operator=( const AssetManager& ) = delete;
  AssetManager( AssetManager&& ) = delete;
  AssetManager& operator=( AssetManager&& ) = delete;

  void parseVertices( cgltf_primitive& primitive,
                      std::vector<glm::vec3>& positions,
                      std::vector<glm::vec3>& normals,
                      std::vector<glm::vec2>& tex_coords,
                      std::vector<glm::ivec4>& jointIndices,
                      std::vector<glm::vec4>& weights,
                      const glm::mat4& transformation ) const;

  auto getSkeleton( cgltf_data* data, cgltf_skin& skin, const glm::mat4& globalTransform )
    -> kogayonon_resources::Skeleton;
  auto isJointInSkin( const cgltf_skin& skin, const cgltf_node* node ) -> bool;
  auto getJointTransform( const cgltf_node* node ) -> kogayonon_resources::JointTransform;
  auto getJointId( const cgltf_data* data, const cgltf_node* node ) -> uint32_t;
  auto getInverseBindMatrices( const cgltf_data*, const cgltf_skin& skin ) -> std::vector<glm::mat4>;

  /**
   * @brief Traverse entire tree from the bone to the root node and calculate the global pose matrix recursively
   * @param bone
   * @return
   */
  auto getGlobalPose( kogayonon_resources::Joint& bone ) -> glm::mat4;
  auto calculateGlobalPoses( kogayonon_resources::Skeleton& skeleton );

  void parseAnimations( cgltf_primitive& primitive );

  void parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const;

  void parseTextures( const cgltf_material* material, std::vector<kogayonon_resources::Texture*>& textures );

  std::thread m_watchThread{};
  std::mutex m_assetMutex{};

  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>> m_loadedTextures;
  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Mesh>> m_loadedMeshes;
};
} // namespace kogayonon_utilities
