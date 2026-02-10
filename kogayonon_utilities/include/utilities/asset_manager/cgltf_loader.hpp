#pragma once
#include <cgltf.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace kogayonon_resources
{
class Mesh;
class Texture;
} // namespace kogayonon_resources

namespace kogayonon_utilities
{
class CgltfLoader
{
public:
  CgltfLoader();
  ~CgltfLoader();

  void loadMesh( const std::string& path,
                 kogayonon_resources::Mesh* m,
                 std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>>& loadedTex );

private:
  void parseAnimations( cgltf_primitive& primitive );
  void parseTextures( const cgltf_material* material,
                      std::vector<kogayonon_resources::Texture*>& textures,
                      std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>>& loadedTex );
  void parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const;
  void parseVertices( cgltf_primitive& primitive,
                      std::vector<glm::vec3>& positions,
                      std::vector<glm::vec3>& normals,
                      std::vector<glm::vec2>& tex_coords,
                      std::vector<glm::ivec4>& jointIndices,
                      std::vector<glm::vec4>& weights,
                      const glm::mat4& transformation ) const;

private:
  auto readFile( const std::string& path ) -> cgltf_data*;

private:
};
} // namespace kogayonon_utilities
