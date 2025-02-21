#include <glad/glad.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb/stb_image.h>
#include "core/model_loader/model_loader.h"
#include "core/logger.h"
#include "core/model_loader/model.h"
#include <chrono>
#include <map>

namespace kogayonon
{
  void ModelLoader::buildModel(std::string path, std::vector<Mesh>& meshes, std::map<std::string, Texture>& textures_loaded)
  {
    m_current_model_path = path.substr(0, path.find_last_of('/'));
    // since i place the models in the models folder i'll do this
    assert(m_current_model_path.size() > 6);
    Logger::logInfo("Loading model file:", path);
    getScene(path);

    // we start processing mesh nodes one by one recursively
    processNode(meshes, m_scene->mRootNode, textures_loaded);
  }

  void ModelLoader::getScene(std::string& path)
  {
    unsigned int importOptions = aiProcess_Triangulate
      | aiProcess_OptimizeMeshes
      | aiProcess_JoinIdenticalVertices
      | aiProcess_CalcTangentSpace;

    m_scene = m_importer.ReadFile(path, importOptions);
    assert(m_scene != nullptr);
  }

  Mesh ModelLoader::processMesh(const aiMesh* mesh, std::map<std::string, Texture>& textures_loaded)
  {
    return Mesh(std::move(getVertices(mesh)), std::move(getIndices(mesh)), std::move(getTextures(mesh, textures_loaded)));
  }

  void ModelLoader::processNode(std::vector<Mesh>& meshes, aiNode* node, std::map<std::string, Texture>& loaded_textures)
  {
    // get all the meshes from this current node
    if (node->mNumMeshes > 0 && node->mMeshes)
    {
      for (unsigned int i = 0; i < node->mNumMeshes; i++)
      {
        aiMesh* mesh = m_scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, loaded_textures));
      }
    }
    else
    {
      Logger::logError("Node had no meshes...");
    }

    // recursively go to the next node and process those meshes too
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      processNode(meshes, node->mChildren[i], loaded_textures);
    }
  }

  std::vector<Texture> ModelLoader::loadMaterialTextures(std::map<std::string, Texture>& textures_loaded, aiMaterial* material, aiTextureType type, std::string type_name)
  {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
      aiString str;
      material->GetTexture(type, i, &str);
      std::string texture_path = str.C_Str();

      // Check if the texture is already loaded before trying to load it
      if (textureAlreadyLoaded(texture_path, textures_loaded))
      {
        Logger::logError("Texture already loaded from -> ", texture_path);
        // we retrieve the already loaded texture in here
        textures.push_back(textures_loaded[texture_path]);
      }
      else
      {
        std::string filename = std::string(texture_path);
        filename = m_current_model_path + '/' + filename;

        int width, height, num_components;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &num_components, 0);
        if (!data)
        {
          Logger::logError("Failed to load texture at path: ", filename);
        }
        else
        {
          Logger::logInfo("Loaded texture ", filename, " [", width, "x", height, "] with ", num_components, " components");
        }
        Texture texture(type_name, texture_path, width, height, num_components, data, true);
        textures.push_back(texture);
        Logger::logInfo("Loaded texture from ", texture.path);
        textures_loaded[texture.path] = texture;
      }
    }
    return textures;
  }

  std::vector<Texture> ModelLoader::getTextures(const aiMesh* mesh, std::map<std::string, Texture>& textures_loaded)
  {
    aiMaterial* material = m_scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture> textures;

    std::vector<Texture> diffuseMaps = loadMaterialTextures(textures_loaded, material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps = loadMaterialTextures(textures_loaded, material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    std::vector<Texture> normalMaps = loadMaterialTextures(textures_loaded, material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<Texture> heightMaps = loadMaterialTextures(textures_loaded, material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return textures;
  }

  bool ModelLoader::textureAlreadyLoaded(const std::string& path, const std::map<std::string, Texture>& loaded_textures)
  {
    return loaded_textures.find(path) != loaded_textures.end();
  }

  std::vector<Vertex> ModelLoader::getVertices(const aiMesh* mesh)
  {
    std::vector<Vertex> vertices;
    vertices.resize(mesh->mNumVertices);

    // fill vertices directly
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      vertices[i].position.x = mesh->mVertices[i].x;
      vertices[i].position.y = mesh->mVertices[i].y;
      vertices[i].position.z = mesh->mVertices[i].z;

      if (mesh->HasNormals())
      {
        vertices[i].normal.x = mesh->mNormals[i].x;
        vertices[i].normal.y = mesh->mNormals[i].y;
        vertices[i].normal.z = mesh->mNormals[i].z;
      }
      if (mesh->mTextureCoords[0])
      {
        vertices[i].texture.x = mesh->mTextureCoords[0][i].x;
        vertices[i].texture.y = mesh->mTextureCoords[0][i].y;

        if (mesh->HasTangentsAndBitangents())
        {
          vertices[i].bitangent.x = mesh->mBitangents[i].x;
          vertices[i].bitangent.y = mesh->mBitangents[i].y;

          vertices[i].tangent.x = mesh->mTangents[i].x;
          vertices[i].tangent.y = mesh->mTangents[i].y;
        }

      }
      else
      {
        vertices[i].texture.x = 0.0f;
        vertices[i].texture.y = 0.0f;
      }
    }
    return vertices;
  }

  std::vector<unsigned int> ModelLoader::getIndices(const aiMesh* mesh)
  {
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace& face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++)
      {
        indices.push_back(face.mIndices[j]);
      }
    }
    return indices;
  }
}
