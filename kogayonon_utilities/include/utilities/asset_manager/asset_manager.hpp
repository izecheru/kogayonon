#pragma once
#include <glm/glm.hpp>
#include <mutex>
#include <string>
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
  AssetManager() = default;
  ~AssetManager() = default;

  // Textures
  std::weak_ptr<kogayonon_resources::Texture> addTexture( const std::string& textureName,
                                                          const std::string& texturePath );
  std::weak_ptr<kogayonon_resources::Texture> addTextureFromMemory( const std::string& textureName,
                                                                    const unsigned char* data );
  std::weak_ptr<kogayonon_resources::Texture> getTexture( const std::string& textureName );

  // Models
  std::weak_ptr<kogayonon_resources::Model> addModel( const std::string& modelName, const std::string& modelPath );
  std::weak_ptr<kogayonon_resources::Model> getModel( const std::string& modelName );

private:
  void parseVertices( cgltf_primitive& primitive, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
                      std::vector<glm::vec2>& tex_coords, const glm::mat4& transformation );
  void parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices );
  void parseTextures( const cgltf_material* material, std::vector<unsigned int>& textureIDs );

private:
  std::mutex m_assetMutex;
  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>> m_loadedTextures;
  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Model>> m_loadedModels;
};
} // namespace kogayonon_utilities