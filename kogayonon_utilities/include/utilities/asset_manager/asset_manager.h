#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include "resources/model.h"
#include "resources/texture.h"

namespace kogayonon_utilities
{
class AssetManager
{
  public:
    AssetManager() = default;
    ~AssetManager() = default;

    // Textures
    std::weak_ptr<kogayonon_resources::Texture> addTexture(const std::string& textureName,
                                                           const std::string& texturePath);
    /**
     * @brief Adding a texture from memory
     * @param textureName The name of the texture
     * @param data The data loaded in memory
     * @return A non-owning ptr to the texture
     */
    std::weak_ptr<kogayonon_resources::Texture> addTextureFromMemory(const std::string& textureName,
                                                                     const unsigned char* data);
    std::weak_ptr<kogayonon_resources::Texture> getTexture(const std::string& textureName);

    // Models
    std::weak_ptr<kogayonon_resources::Model> addModel(const std::string& modelName, const std::string& modelPath);
    std::weak_ptr<kogayonon_resources::Model> getModel(const std::string& modelName);

  private:
    std::mutex m_assetMutex;
    std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>> m_loadedTextures;
    std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Model>> m_loadedModels;
};
} // namespace kogayonon_utilities