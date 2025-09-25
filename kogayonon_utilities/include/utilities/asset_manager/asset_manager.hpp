#pragma once
#include <filesystem>
#include <glm/glm.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include "resources/model.hpp"
#include "resources/texture.hpp"

struct cgltf_primitive;
struct cgltf_accessor;
struct cgltf_material;

namespace kogayonon_utilities
{
class AssetManager
{
public:
  AssetManager();
  ~AssetManager();

  // Textures
  std::weak_ptr<kogayonon_resources::Texture> addTexture( const std::string& textureName,
                                                          const std::string& texturePath );
  std::weak_ptr<kogayonon_resources::Texture> addTextureWithoutParams( const std::string& textureName,
                                                                       const std::string& texturePath );

  std::weak_ptr<kogayonon_resources::Texture> addTextureFromMemory( const std::string& textureName,
                                                                    const unsigned char* data );
  std::weak_ptr<kogayonon_resources::Texture> getTexture( const std::string& textureName );
  /**
   * @brief Deletes a texture from the loaded map, even though we index with texture name which is not actual filename,
   * we will loop through the map with an iterator it and look if the path == it->second->getPath() since we store the
   * path in the texture object
   * @param path Path of the texture file
   */
  void removeTexture( const std::string& path );

  // Models
  std::weak_ptr<kogayonon_resources::Model> addModel( const std::string& modelName, const std::string& modelPath );
  std::weak_ptr<kogayonon_resources::Model> getModel( const std::string& modelName );

private:
  void parseVertices( cgltf_primitive& primitive, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
                      std::vector<glm::vec2>& tex_coords, const glm::mat4& transformation ) const;

  void parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const;

  void parseTextures( const cgltf_material* material, std::vector<unsigned int>& textureIDs );

  /**
   * @brief Uploads each mesh data to the gpu and tells it how to interpret every buffer
   * @param meshes A vector of meshes that will need to be prepared for rendering
   */
  void prepareMeshes( std::vector<kogayonon_resources::Mesh>& meshes ) const;

  std::thread m_watchThread{};
  std::mutex m_assetMutex{};

  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>> m_loadedTextures;
  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Model>> m_loadedModels;
};
} // namespace kogayonon_utilities