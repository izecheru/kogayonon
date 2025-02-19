#pragma once
#include <assimp/Importer.hpp>
#include "core/model_loader/model.h"

namespace kogayonon
{
  class ModelLoader :public Singleton<ModelLoader> {
  public:

      static void buildModel(std::string path, std::vector<Mesh>& meshes, std::map<std::string, Texture>& textures_loaded);

    static void getScene(std::string& path);

    static Mesh processMesh(const aiMesh* mesh, std::map<std::string, Texture>& textures_loaded);
    static void processNode(std::vector<Mesh>& meshes, aiNode* node, std::map<std::string, Texture>& loaded_textures);
    static unsigned int textureFromFile(std::string& path, const std::string& directory, bool gamma);
    static std::vector<Texture> loadMaterialTextures(std::map<std::string, Texture>& textures_loaded, aiMaterial* material, aiTextureType type, std::string type_name);

    static std::vector<Texture> getTextures(const aiMesh* mesh, std::map<std::string, Texture>& textures_loaded);
    static bool textureAlreadyLoaded(const std::string& path, const std::map<std::string, Texture>& loaded_textures);
    static std::vector<Vertex> getVertices(const aiMesh* mesh);
    static std::vector<unsigned int> getIndices(const aiMesh* mesh);

  private:
    // assimp importer member variable to load objects into the game
    inline static Assimp::Importer m_importer;
    inline static const aiScene* m_scene = nullptr;
    inline static std::string m_current_model_path;
  };
}
