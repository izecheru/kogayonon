#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <cgltf/cgltf.h>

#include "core/singleton/singleton.h"
#include "model.h"

namespace kogayonon
{
  class ModelLoader :public Singleton<ModelLoader>
  {
  public:
    bool pushModel(const cgltf_data* data, const std::string& model_path, std::function<void(Model&)>callback, std::mutex& t_mutex, std::unordered_map<std::string, Model>& models_map);
    void assignModelMeshes(const cgltf_data* data, std::vector<Mesh>& meshes);

    void parsePrimitives(cgltf_node& node, std::vector<uint32_t>& indices, std::vector<Vertex>& final_vertices);
    void parseVertices(cgltf_primitive& primitive, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& tex_coords, const glm::mat4& transformation)const;
    void parseIndices(cgltf_accessor* accessor, std::vector<uint32_t>& indices)const;
  };
}
