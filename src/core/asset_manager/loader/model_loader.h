#pragma once

#include <cgltf.h>

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

#include "core/singleton/singleton.h"
#include "model.h"

namespace kogayonon
{
  class ModelLoader : public Singleton<ModelLoader>
  {
  public:
    void assignModelMeshes(const cgltf_data* data, std::vector<Mesh>& meshes);
    void parseIndices(cgltf_accessor* accessor, std::vector<uint32_t>& indices) const;
    void parseTextures(const cgltf_material* material, std::vector<std::string>& texture_paths) const;

    void parsePrimitives(cgltf_node& node, std::vector<uint32_t>& indices, std::vector<Vertex>& final_vertices,
                         std::vector<std::string>& texture_paths);
    void parseVertices(cgltf_primitive& primitive, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
                       std::vector<glm::vec2>& tex_coords, const glm::mat4& transformation) const;
  };
} // namespace kogayonon