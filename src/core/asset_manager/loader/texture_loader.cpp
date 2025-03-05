#include <filesystem>
#include <stb/stb_image.h>

#include "core/asset_manager/loader/texture_loader.h"
#include "core/task/task_manager.h"
#include "core/time_tracker/time_tracker.h"

namespace kogayonon
{
  void TextureLoader::loadTexture(const cgltf_texture* texture, std::unordered_map<std::string, Texture>& loaded_textures, const std::string& type, const std::string& model_dir)
  {
    if(texture && texture->image && texture->image->uri)
    {
      std::string texture_path = (std::filesystem::path(model_dir) / texture->image->uri).lexically_normal().string();

      if(loaded_textures.find(texture_path) != loaded_textures.end())
      {
        return;
      }
      else
      {

        Logger::logInfo("Loading texture from:", texture_path);

        int w; int h; int num_comp;
        unsigned char* tex_data = stbi_load(texture_path.c_str(), &w, &h, &num_comp, 0);

        if(!tex_data)
        {
          Logger::logError("Failed to load texture:", texture_path);
          return;
        }

        std::vector<unsigned char> texture_data(tex_data, tex_data + (w * h * num_comp));
        Texture tex(type, texture_path, w, h, num_comp, texture_data, true);
        loaded_textures[texture_path] = tex;
      }
    }
  }

  void TextureLoader::pushTexture(const std::string& texture_path,
    std::function<void(const Texture&)> callback,
    std::mutex& mutex,
    std::unordered_map<std::string, Texture>& textures,
    const cgltf_data* data)
  {
    TimeTracker::getInstance().startCount("texture");
    TaskManager::getInstance().enqueue([this, &mutex, &textures, callback, texture_path, data]()
      {
        if(textures.find(texture_path) != textures.end()) return;

        Texture tex;
        std::lock_guard lock(mutex);
        initializeTextures(data, texture_path, textures);
        textures[texture_path] = tex;
      });
  }

  void TextureLoader::initializeTextures(const cgltf_data* data, const std::string& model_dir, std::unordered_map<std::string, Texture>& loaded_textures)
  {
    for(size_t node_index = 0; node_index < data->nodes_count; node_index++)
    {
      cgltf_node const& node = data->nodes[node_index];

      if(!node.mesh) continue;

      cgltf_mesh const& mesh = *node.mesh;

      for(size_t primitive_index = 0; primitive_index < mesh.primitives_count; primitive_index++)
      {
        cgltf_primitive const& primitive = mesh.primitives[primitive_index];
        if(primitive.material)
        {
          processMaterial(primitive.material, loaded_textures, model_dir);
        }
      }
    }
  }
  void TextureLoader::processMaterial(const cgltf_material* material, std::unordered_map<std::string, Texture>& loaded_textures, const std::string& model_dir)
  {
    if(!material)return;
    // Base Color (Diffuse)
    if(material->has_pbr_metallic_roughness)
    {
      loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, loaded_textures, "texture_diffuse", model_dir);
      loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, loaded_textures, "texture_metallic_roughness", model_dir);
    }

    // Specular-Glossiness Workflow
    if(material->has_pbr_specular_glossiness)
    {
      loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, loaded_textures, "texture_specular", model_dir);
      loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, loaded_textures, "texture_glossiness", model_dir);
    }
    // Normal, Occlusion, Emissive, and Transmission Maps
    loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, loaded_textures, "texture_normal", model_dir);
    loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, loaded_textures, "texture_occlusion", model_dir);
    loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, loaded_textures, "texture_emissive", model_dir);
    // Transmission Map
    if(material->has_transmission)
    {
      loadTexture(material->pbr_metallic_roughness.metallic_roughness_texture.texture, loaded_textures, "texture_transmission", model_dir);
    }
  }
}