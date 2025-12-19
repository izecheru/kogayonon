#pragma once
#include <filesystem>
#include <glm/glm.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include "resources/mesh.hpp"
#include "resources/texture.hpp"

struct cgltf_primitive;
struct cgltf_accessor;
struct cgltf_material;

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
  kogayonon_resources::Texture* addTexture( const std::string& textureName );
  kogayonon_resources::Texture* addTexture( const std::string& textureName, const std::string& texturePath );
  std::weak_ptr<kogayonon_resources::Texture> addTextureWithoutParams( const std::string& textureName,
                                                                       const std::string& texturePath );

  std::weak_ptr<kogayonon_resources::Texture> addTextureFromMemory( const std::string& textureName,
                                                                    const unsigned char* data );

  /**
   * @brief Finds a texture by name
   * @param textureName Name of the texture you are looking for
   * @param folder Folder we retrieve the texture from, defaults to "resources/textures/" if not specified as param
   * @return A weak ptr to the texture found
   */
  std::weak_ptr<kogayonon_resources::Texture> getTextureByName( const std::string& textureName,
                                                                const std::string& folder = "resources/textures/" );

  /**
   * @brief Deletes a texture from the loaded map, even though we index with texture name which is not actual filename,
   * we will loop through the map with an iterator it and look if the path == it->second->getPath() since we store the
   * path in the texture object
   * @param path Path of the texture file
   */
  void removeTexture( const std::string& path );

  std::weak_ptr<kogayonon_resources::Texture> getTextureById( uint32_t id );

  // Models
  kogayonon_resources::Mesh* addMesh( const std::string& meshName, const std::string& meshPath );
  kogayonon_resources::Mesh* addMesh( const std::string& meshName );
  kogayonon_resources::Mesh* getMesh( const std::string& meshPath );

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

  void parseVertices( cgltf_primitive& primitive, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
                      std::vector<glm::vec2>& tex_coords, const glm::mat4& transformation ) const;

  void parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const;

  void parseTextures( const cgltf_material* material, std::vector<kogayonon_resources::Texture*>& textures );

  std::thread m_watchThread{};
  std::mutex m_assetMutex{};

  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>> m_loadedTextures;
  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Mesh>> m_loadedMeshes;
};
} // namespace kogayonon_utilities