#include <glad/glad.h>
#include "core/model_loader/model_loader.h"
#include "core/logger.h"
#include "core/model_loader/model.h"
#include <fstream>
#include <regex>

namespace kogayonon
{
  void ModelLoader::buildModel(const std::string& path, std::vector<Mesh>& meshes)
  {
    std::vector<Vertex> final_vertices;
    std::vector<uint32_t> indices;
    std::vector<Texture> textures;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> tex_coords;
    std::vector<glm::vec3> normals;
    std::map<std::string, uint32_t> unique_vertices; // Persist across faces

    std::ifstream in(path);
    assert(in.is_open());
    std::string buffer;

    while (std::getline(in, buffer))
    {
      buffer.erase(std::remove(buffer.begin(), buffer.end(), '\r'), buffer.end());
      std::string current_mesh_name;

      if (buffer.rfind("o ", 0) == 0)
      {
        std::istringstream iss(buffer.substr(2));
        iss >> current_mesh_name;
        if (!final_vertices.empty() && !indices.empty())
        {
          meshes.push_back(Mesh(final_vertices, indices, textures));
        }
        final_vertices.clear();
        indices.clear();
        unique_vertices.clear();
      }

      if (buffer.rfind("v ", 0) == 0)
      {
        std::istringstream iss(buffer.substr(2));
        float x, y, z;
        iss >> x >> y >> z;
        vertices.push_back({ x, y, z });
      }

      if (buffer.rfind("vt ", 0) == 0)
      {
        std::istringstream iss(buffer.substr(3));
        float u, v;
        iss >> u >> v;
        tex_coords.push_back({ u, v });
      }

      if (buffer.rfind("vn ", 0) == 0)
      {
        std::istringstream iss(buffer.substr(3));
        float x, y, z;
        iss >> x >> y >> z;
        normals.push_back({ x, y, z });
      }

      if (buffer.rfind("f ", 0) == 0)
      {
        std::istringstream iss(buffer.substr(2));
        std::vector<uint32_t> face_indices;

        std::string face_element;
        while (iss >> face_element)
        {
          std::istringstream face_stream(face_element);
          std::string vertex_index, tex_index, normal_index;
          std::getline(face_stream, vertex_index, '/');
          std::getline(face_stream, tex_index, '/');
          std::getline(face_stream, normal_index, '/');

          uint32_t v_idx = std::stoi(vertex_index) - 1;
          uint32_t t_idx = (tex_index.empty() || std::stoi(tex_index) - 1 >= tex_coords.size()) ? 0 : std::stoi(tex_index) - 1;
          uint32_t n_idx = (normal_index.empty() || std::stoi(normal_index) - 1 >= normals.size()) ? 0 : std::stoi(normal_index) - 1;

          std::string key = vertex_index + "/" + tex_index + "/" + normal_index;
          if (unique_vertices.find(key) == unique_vertices.end())
          {
            Vertex final_vertex;
            final_vertex.position = vertices[v_idx];
            final_vertex.texture = tex_coords[t_idx];
            final_vertex.normal = normals[n_idx];

            final_vertices.push_back(final_vertex);
            unique_vertices[key] = final_vertices.size() - 1;
          }

          face_indices.push_back(unique_vertices[key]);
        }

        // Triangulate faces
        for (size_t i = 1; i + 1 < face_indices.size(); ++i)
        {
          indices.push_back(face_indices[0]);
          indices.push_back(face_indices[i]);
          indices.push_back(face_indices[i + 1]);
        }
      }
    }

    // Push last mesh if exists
    if (!final_vertices.empty() && !indices.empty())
    {
      meshes.push_back(Mesh(final_vertices, indices, textures));
    }

    in.close();
  }
}
