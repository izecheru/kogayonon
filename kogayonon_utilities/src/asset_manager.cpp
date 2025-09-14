#include "utilities/asset_manager/asset_manager.h"
#include <SOIL2/SOIL2.h>
#include <assert.h>
#include <filesystem>
#include "logger/logger.h"
#include "resources/model.h"
#include "resources/texture.h"
using namespace kogayonon_logger;

namespace kogayonon_utilities
{
std::weak_ptr<kogayonon_resources::Texture> AssetManager::addTexture(const std::string& textureName,
                                                                     const std::string& texturePath)
{
    if (auto& it = m_loadedTextures.find(textureName); it != m_loadedTextures.end())
    {
        Logger::info("We already have the texture: ", textureName, " from ", texturePath);
        return it->second;
    }

    std::lock_guard lock(m_assetMutex);
    assert(std::filesystem::exists(texturePath) && "Texture file does not exist");

    auto id = SOIL_load_OGL_texture(texturePath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    auto tex = std::make_shared<kogayonon_resources::Texture>(id, texturePath, 0, 0, 0);
    m_loadedTextures.emplace(textureName, std::move(tex));
    return m_loadedTextures.at(textureName);
}

std::weak_ptr<kogayonon_resources::Model> AssetManager::addModel(const std::string& modelName,
                                                                 const std::string& modelPath)
{
    if (auto& it = m_loadedModels.find(modelName); it != m_loadedModels.end())
    {
        Logger::info("We already have the texture: ", modelName, " from ", modelPath);
        return it->second;
    }

    std::lock_guard lock(m_assetMutex);
    assert(std::filesystem::exists(modelPath) && "Model file does not exist");
    return m_loadedModels.at(modelName);
}

std::weak_ptr<kogayonon_resources::Texture> AssetManager::addTextureFromMemory(const std::string& textureName,
                                                                               const unsigned char* data)
{
    return std::weak_ptr<kogayonon_resources::Texture>();
}

std::weak_ptr<kogayonon_resources::Texture> AssetManager::getTexture(const std::string& textureName)
{
    auto& it = m_loadedTextures.find(textureName);
    assert(it != m_loadedTextures.end() && "texture must be in the map");
    return m_loadedTextures.at(textureName);
}

std::weak_ptr<kogayonon_resources::Model> kogayonon_utilities::AssetManager::getModel(const std::string& modelName)
{
    auto& it = m_loadedModels.find(modelName);
    assert(it != m_loadedModels.end() && "model must be in the map");
    return m_loadedModels.at(modelName);
}
} // namespace kogayonon_utilities