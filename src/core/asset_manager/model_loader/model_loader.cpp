
#include <glad/glad.h>
#include "core/asset_manager/model_loader/model_loader.h"
#include "core/logger.h"
#include "core/asset_manager/model_loader/model.h"
#include <fstream>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stb/stb_image.h>
#include <filesystem>

namespace kogayonon
{
  void ModelLoader::loadTexture(cgltf_texture* texture, const std::string& type, std::vector<Texture>& textures, const std::string& model_dir)
  {
    if (texture && texture->image && texture->image->uri)
    {
      std::string texture_path = (std::filesystem::path(model_dir) / texture->image->uri).lexically_normal().string();

      Logger::logInfo("Loading texture from:", texture_path);

      int w, h, num_comp;
      //stbi_set_flip_vertically_on_load(true);
      unsigned char* tex_data = stbi_load(texture_path.c_str(), &w, &h, &num_comp, 0);

      if (!tex_data)
      {
        Logger::logError("Failed to load texture:", texture_path);
        return;
      }

      std::vector<unsigned char> texture_data(tex_data, tex_data + (w * h * num_comp));
      textures.push_back(Texture(type, texture_path, w, h, num_comp, texture_data, true));
    }
  }

  void ModelLoader::buildModel(const std::string& path, std::vector<Mesh>& meshes)
  {
    std::string model_dir = std::filesystem::path(path).parent_path().string();
    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
    if (result != cgltf_result_success)
    {
      Logger::logError("Failed to load glTF file:", path);
      return;
    }

    cgltf_load_buffers(&options, data, path.c_str());

    std::vector<Vertex> final_vertices;
    std::vector<uint32_t> indices;
    std::vector<Texture> textures;

    // Iterate through nodes to get their transformations
    for (size_t node_index = 0; node_index < data->nodes_count; node_index++)
    {
      cgltf_node& node = data->nodes[node_index];

      if (!node.mesh) continue; // Skip if node has no mesh

      cgltf_mesh& mesh = *node.mesh;

      // Compute transformation matrix for the node
      glm::mat4 transform = glm::mat4(1.0f);
      if (node.has_matrix)
      {
        transform = glm::make_mat4(node.matrix);
      }
      else
      {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
        glm::mat4 rotation = glm::mat4_cast(glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]));
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
        transform = translation * rotation * scale;
      }

      for (size_t primitive_index = 0; primitive_index < mesh.primitives_count; primitive_index++)
      {
        cgltf_primitive& primitive = mesh.primitives[primitive_index];

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> tex_coords;
        std::vector<glm::vec3> normals;

        // Extract vertex attributes
        for (size_t attr_index = 0; attr_index < primitive.attributes_count; attr_index++)
        {
          cgltf_attribute& attribute = primitive.attributes[attr_index];
          cgltf_accessor* accessor = attribute.data;
          cgltf_buffer_view* buffer_view = accessor->buffer_view;
          uint8_t* buffer_data = (uint8_t*)buffer_view->buffer->data + buffer_view->offset;

          size_t vertex_count = accessor->count;
          size_t stride = buffer_view->stride ? buffer_view->stride : cgltf_calc_size(accessor->type, accessor->component_type);

          for (size_t v = 0; v < vertex_count; v++)
          {
            float* data = (float*)(buffer_data + accessor->offset + v * stride);

            if (attribute.type == cgltf_attribute_type_position)
            {
              glm::vec3 position(data[0], data[1], data[2]);

              // Apply the node transformation
              position = glm::vec3(transform * glm::vec4(position, 1.0f));

              positions.push_back(position);
            }
            else if (attribute.type == cgltf_attribute_type_texcoord)
            {
              tex_coords.push_back(glm::vec2(data[0], data[1]));
            }
            else if (attribute.type == cgltf_attribute_type_normal)
            {
              glm::vec3 normal(data[0], data[1], data[2]);

              // Transform normal correctly (ignore translation)
              normal = glm::normalize(glm::mat3(transform) * normal);

              normals.push_back(normal);
            }
          }
        }

        // Extract index data
        if (primitive.indices)
        {
          cgltf_accessor* accessor = primitive.indices;
          cgltf_buffer_view* buffer_view = accessor->buffer_view;
          uint8_t* buffer_data = (uint8_t*)buffer_view->buffer->data + buffer_view->offset;

          for (size_t i = 0; i < accessor->count; i++)
          {
            uint32_t index = 0;
            if (accessor->component_type == cgltf_component_type_r_16u)
            {
              index = *((uint16_t*)(buffer_data + accessor->offset + i * sizeof(uint16_t)));
            }
            else if (accessor->component_type == cgltf_component_type_r_32u)
            {
              index = *((uint32_t*)(buffer_data + accessor->offset + i * sizeof(uint32_t)));
            }
            else if (accessor->component_type == cgltf_component_type_r_8u)
            {
              index = *((uint8_t*)(buffer_data + accessor->offset + i * sizeof(uint8_t)));
            }
            indices.push_back(index);
          }
        }

        // Store transformed vertices
        for (size_t i = 0; i < positions.size(); i++)
        {
          Vertex final_vertex;
          final_vertex.position = positions[i];
          final_vertex.texture = i < tex_coords.size() ? tex_coords[i] : glm::vec2(0.0f);
          final_vertex.normal = i < normals.size() ? normals[i] : glm::vec3(0.0f);

          final_vertices.push_back(final_vertex);
        }


        // Load textures from material
        if (primitive.material)
        {
          cgltf_material* material = primitive.material;

          // Base Color (Diffuse)
          if (material->has_pbr_metallic_roughness)
          {
            loadTexture(material->pbr_metallic_roughness.base_color_texture.texture, "texture_diffuse", textures, model_dir);
            loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, "texture_metallic_roughness", textures, model_dir);
          }

          // Specular-Glossiness Workflow
          if (material->has_pbr_specular_glossiness)
          {
            loadTexture(material->pbr_specular_glossiness.diffuse_texture.texture, "texture_specular", textures, model_dir);
            loadTexture(material->pbr_specular_glossiness.specular_glossiness_texture.texture, "texture_glossiness", textures, model_dir);
          }

          // Normal, Occlusion, Emissive, and Transmission Maps
          loadTexture(material->normal_texture.texture, "texture_normal", textures, model_dir);
          loadTexture(material->occlusion_texture.texture, "texture_occlusion", textures, model_dir);
          loadTexture(material->emissive_texture.texture, "texture_emissive", textures, model_dir);
          if (material->has_transmission)
          {
            loadTexture(material->transmission.transmission_texture.texture, "texture_transmission", textures, model_dir);
          }
        }
      }

      // Store mesh
      if (!final_vertices.empty() && !indices.empty())
      {
        meshes.push_back(Mesh(std::move(final_vertices), std::move(indices), textures));
      }
      final_vertices.clear();
      indices.clear();
    }

    cgltf_free(data);
  }
}
