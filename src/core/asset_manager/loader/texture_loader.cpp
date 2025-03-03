#include "core/asset_manager//loader/texture_loader.h"
#include <filesystem>
#include <stb/stb_image.h>

namespace kogayonon
{
  void TextureLoader::loadTexture(cgltf_texture* texture, const std::string& type, std::vector<Texture>& textures, const std::string& model_dir)
  {
    if (texture && texture->image && texture->image->uri)
    {
      std::string texture_path = (std::filesystem::path(model_dir) / texture->image->uri).lexically_normal().string();

      if (m_loaded_textures.find(texture_path) != m_loaded_textures.end())
      {
        return;
      }
      else
      {

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
        Texture tex(type, texture_path, w, h, num_comp, texture_data, true);
        // TODO mesh should just get the path and call getTexture() or something, at least i THINK so
        textures.push_back(tex);
        m_loaded_textures[texture_path] = tex;
      }
    }
  }

}