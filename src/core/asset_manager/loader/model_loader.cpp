#include <assert.h>
#include <cgltf/cgltf.h>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

#include "core/asset_manager/loader/model_loader.h"
#include "core/asset_manager/loader/texture_loader.h"
#include "core/serialize/mesh_serializer.h"
#include "core/task/task_manager.h"

namespace kogayonon
{
  bool ModelLoader::pushModel(const cgltf_data* data, const std::string& model_path, std::function<void(Model&)> callback, std::mutex& mutex, std::unordered_map<std::string, Model>& models_map)
  {
    // make sure model file exists otherwise bad luck
    assert(data != nullptr);

    // construct the directory for the serialized model
    std::filesystem::path m_p(model_path);
    std::string model_name = m_p.stem().string();

    // all models should reside in serialized_models like so /<model_name>.bin
    std::filesystem::path model_dir = m_p.parent_path() / "serialized_models";
    if(!std::filesystem::exists(model_dir))
    {
      std::filesystem::create_directories(model_dir);
    }

    if(std::filesystem::path bin_path = model_dir / (model_name + ".bin"); std::filesystem::exists(bin_path))
    {
      // we already have a serialized model so we deserialize it and assing meshes
      TaskManager::getInstance().enqueue([bin_path, &mutex, &models_map, model_path, callback]()
        {
          Model model;
          {
            MeshSerializer& serializer = MeshSerializer::getInstance();
            std::scoped_lock lock(mutex, serializer.getMutex());
            std::ifstream in{};
            serializer.deserializeMeshes(bin_path.string(), model, in);
            models_map[model_path] = model;
          }
          callback(models_map[model_path]);
        });
    }
    else // needs to be serialized
    {
      TaskManager::getInstance().enqueue([this, data, bin_path, &mutex, &models_map, model_path, callback]()
        {
          Model model;
          assignModelMeshes(data, model.getMeshes());
          KLogger::log(LogType::INFO, bin_path.string());
          {
            MeshSerializer& serializer = MeshSerializer::getInstance();
            std::scoped_lock lock(mutex, serializer.getMutex());
            std::ofstream out{};
            serializer.serializeMeshes(bin_path.string(), model, out);
            models_map[model_path] = model;
          }
          callback(models_map[model_path]);
        });
    }

    return true;
  }

  void ModelLoader::assignModelMeshes(const cgltf_data* data, std::vector<Mesh>& meshes)
  {
    std::vector<Vertex> final_vertices;
    std::vector<uint32_t> indices;

    for(size_t node_index = 0; node_index < data->nodes_count; node_index++)
    {
      parsePrimitives(data->nodes[node_index], indices, final_vertices);

      // Store mesh
      if(!final_vertices.empty() && !indices.empty())
      {
        meshes.emplace_back(final_vertices, indices);
      }
      final_vertices.clear();
      indices.clear();
    }

    // parse animations
    for(size_t animation_index = 0; animation_index < data->animations_count; animation_index++)
    {
      cgltf_animation& animation = data->animations[animation_index];
      KLogger::log(LogType::INFO, "Parsing animation:", animation.name);
    }
  }

  void ModelLoader::parsePrimitives(cgltf_node& node, std::vector<uint32_t>& indices, std::vector<Vertex>& final_vertices)
  {
    if(node.mesh == nullptr) return;

    cgltf_mesh& mesh = *node.mesh;
    auto transformation = glm::mat4(1.0f);
    if(node.has_matrix)
    {
      transformation = glm::make_mat4(node.matrix);
    }
    else
    {
      glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
      glm::mat4 rotation = glm::mat4_cast(glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]));
      glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
      transformation = translation * rotation * scale;
    }

    for(size_t primitive_index = 0; primitive_index < mesh.primitives_count; primitive_index++)
    {
      cgltf_primitive& primitive = mesh.primitives[primitive_index];

      std::vector<glm::vec3> positions;
      std::vector<glm::vec3> normals;
      std::vector<glm::vec2> tex_coords;

      // extract vertex data and texture coords in the vectors declared above
      parseVertices(primitive, positions, normals, tex_coords, transformation);

      // extract index data
      if(primitive.indices)
      {
        parseIndices(primitive.indices, indices);
      }

      // store transformed vertices
      for(size_t i = 0; i < positions.size(); i++)
      {
        Vertex final_vertex;
        final_vertex.position = positions[i];
        final_vertex.tex_coords = i < tex_coords.size() ? tex_coords[i] : glm::vec2(0.0f);
        final_vertex.normal = i < normals.size() ? normals[i] : glm::vec3(0.0f);

        final_vertices.push_back(final_vertex);
      }
    }
  }

  void ModelLoader::parseVertices(cgltf_primitive& primitive, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& tex_coords, const glm::mat4& transformation)const
  {
    // extract vertex attributes
    for(size_t attr_index = 0; attr_index < primitive.attributes_count; attr_index++)
    {
      cgltf_attribute& attribute = primitive.attributes[attr_index];
      cgltf_accessor* accessor = attribute.data;
      cgltf_buffer_view* buffer_view = accessor->buffer_view;
      assert(buffer_view != nullptr);
      uint8_t* buffer_data = (uint8_t*)buffer_view->buffer->data + buffer_view->offset;
      assert(buffer_data != nullptr);

      size_t vertex_count = accessor->count;
      size_t stride = buffer_view->stride ? buffer_view->stride : cgltf_calc_size(accessor->type, accessor->component_type);
      for(size_t v = 0; v < vertex_count; v++)
      {
        auto t_data = (float*)(buffer_data + accessor->offset + v * stride);

        switch(attribute.type)
        {
          case cgltf_attribute_type_position:
            glm::vec3 position(t_data[0], t_data[1], t_data[2]);

            // Apply the node transformation
            position = glm::vec3(transformation * glm::vec4(position, 1.0f));

            positions.push_back(position);
            break;
          case cgltf_attribute_type_normal:
            glm::vec3 normal(t_data[0], t_data[1], t_data[2]);

            // Transform normal correctly (ignore translation)
            normal = glm::normalize(glm::mat3(transformation) * normal);

            normals.push_back(normal);
            break;
          case cgltf_attribute_type_texcoord:
            tex_coords.push_back(glm::vec2(t_data[0], t_data[1]));
            break;
          default:
            break;
        }
      }
    }
  }

  void ModelLoader::parseIndices(cgltf_accessor* accessor, std::vector<uint32_t>& indices)const
  {
    cgltf_buffer_view* buffer_view = accessor->buffer_view;
    uint8_t* buffer_data = (uint8_t*)buffer_view->buffer->data + buffer_view->offset;

    for(size_t i = 0; i < accessor->count; i++)
    {
      uint32_t index = 0;
      if(accessor->component_type == cgltf_component_type_r_16u)
      {
        index = *((uint16_t*)(buffer_data + accessor->offset + i * sizeof(uint16_t)));
      }
      else if(accessor->component_type == cgltf_component_type_r_32u)
      {
        index = *((uint32_t*)(buffer_data + accessor->offset + i * sizeof(uint32_t)));
      }
      else if(accessor->component_type == cgltf_component_type_r_8u)
      {
        index = *((uint8_t*)(buffer_data + accessor->offset + i * sizeof(uint8_t)));
      }
      indices.push_back(index);
    }
  }
}