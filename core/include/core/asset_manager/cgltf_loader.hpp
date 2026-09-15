#pragma once
#include <cgltf.h>
#include <glm/glm.hpp>
#include "precompiled/pch.hpp"

namespace resources
{
class Mesh;
struct Joint;
struct Skeleton;
class Texture;
} // namespace resources

namespace core
{
class CgltfLoader
{
public:
  CgltfLoader();
  ~CgltfLoader();

  void loadMesh( const std::string& path, resources::Mesh* m, std::vector<cgltf_mesh*>& meshes );
  void freeData();

private:
  /**
   * @brief Get gltf index node id
   * @param target
   * @param allNodes
   * @param numNodes
   * @return
   */
  auto getNodeId( cgltf_node* target, cgltf_node* allNodes, unsigned int numNodes ) -> int;

  /**
   * @brief Iterate through all animations and store them
   * @param data
   */
  void parseAnimations( cgltf_data* data, resources::Skeleton& s );

  /**
   * @brief Get all the mesh indices and fill the indices vector
   * @param accessor
   * @param indices
   */
  void parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const;

  /**
   * @brief Parse all the vertices and fills the references
   * @param primitive
   * @param positions
   * @param normals
   * @param tex_coords
   * @param jointIndices
   * @param weights
   * @param transformation
   */
  void parseVertices( cgltf_primitive& primitive,
                      std::vector<glm::vec3>& positions,
                      std::vector<glm::vec3>& normals,
                      std::vector<glm::vec2>& tex_coords,
                      std::vector<glm::ivec4>& jointIndices,
                      std::vector<glm::vec4>& weights,
                      const glm::mat4& transformation ) const;

  /**
   * @brief Iterates over the joints and assigns global transform matrix
   * @param skeleton
   * @return
   */
  auto calculateGlobalMatrix( resources::Skeleton& skeleton );

  /**
   * @brief Recursive function for globalMatrix calculation global(joint) = global(parentJoint) * local(joint)
   * @param joint
   * @return
   */
  auto getGlobalTransform( resources::Joint& joint ) -> glm::mat4;

  /**
   * @brief Loads the skeleton from the cgltf_skin
   * @param data
   * @param skin
   * @return
   */
  auto loadSkeleton( cgltf_data* data, cgltf_skin* skin ) -> resources::Skeleton;

  /**
   * @brief Get all the inverse bind matrices from the accessor
   * @param skin
   * @return
   */
  auto getInverseBindMatrices( cgltf_skin* skin ) -> std::vector<glm::mat4>;
  auto readFile( const std::string& path ) -> cgltf_data*;

  /**
   * @brief Calculate the local matrix from translation * rotation * scale
   * @param node
   * @return
   */
  auto getLocalMatrix( const cgltf_node* node ) -> glm::mat4;

  /**
   * @brief Debug function to see the hierarchical view of joints, it must be 1:1 with the blender one
   * @param joint
   * @param level
   */
  void printChildren( resources::Joint* joint, uint32_t level );

private:
  cgltf_data* m_data;
};
} // namespace core
