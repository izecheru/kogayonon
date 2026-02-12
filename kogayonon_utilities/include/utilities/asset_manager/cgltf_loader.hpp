#pragma once
#include <cgltf.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace kogayonon_resources
{
class Mesh;
struct Joint;
struct Skeleton;
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
  auto getNodeId( cgltf_node* target, cgltf_node* allNodes, unsigned int numNodes ) -> int;

  void parseAnimations( cgltf_data* data );

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

  auto loadSkeleton( cgltf_data* data, cgltf_skin* skin ) -> kogayonon_resources::Skeleton;

  auto getInverseBindMatrices( cgltf_skin* skin ) -> std::vector<glm::mat4>;

  auto readFile( const std::string& path ) -> cgltf_data*;
  auto getLocalMatrix( const cgltf_node* node ) -> glm::mat4;
  auto calculateMatrices( kogayonon_resources::Skeleton& skeleton );

  void printChildren( kogayonon_resources::Joint* joint, uint32_t level );

private:
};
} // namespace kogayonon_utilities
