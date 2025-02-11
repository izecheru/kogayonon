#include <glad/glad.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stbi\stb_image.h>
#include "core/model_loader/model_loader.h"
#include "core/logger.h"
#include "core/renderer/model.h"

namespace kogayonon
{
  void ModelLoader::buildModel(std::string& path, std::vector<Mesh>& meshes, std::vector<Texture>& textures_loaded, Shader& shader) {
    m_current_model_path = path.substr(0, path.find_last_of('/'));
    getScene(path);

    // we start processing mesh nodes one by one recursively
    processNode(meshes, m_scene->mRootNode, textures_loaded, shader);
  }

  void ModelLoader::getScene(std::string& path) {
    unsigned int importOptions = aiProcess_Triangulate
      | aiProcess_OptimizeMeshes
      | aiProcess_JoinIdenticalVertices
      | aiProcess_CalcTangentSpace
      | aiProcess_FlipUVs
      ;
    m_scene = m_importer.ReadFile(path, importOptions);
    if (m_scene == nullptr || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
    {
      Logger::logError("Could not load object from [", path, "]");
    }
  }

  Mesh ModelLoader::processMesh(const aiMesh* mesh, std::vector<Texture>& textures_loaded, Shader& shader) {
    return Mesh(std::move(getVertices(mesh)), std::move(getIndices(mesh)), std::move(getTextures(mesh, textures_loaded)));
  }

  /// <summary>
  ///  Process the current node from the mesh
  /// </summary>
  /// <param name="meshes">Meshes array reference to fill the Model array</param>
  /// <param name="node">Current node we are at</param>
  /// <param name="scene">The scene we get from reading the model file</param>
  /// <param name="loaded_textures">Vector of loaded textures</param>
  void ModelLoader::processNode(std::vector<Mesh>& meshes, aiNode* node, std::vector<Texture>& loaded_textures, Shader& shader) {
    // get all the meshes from this current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh* mesh = m_scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(std::move(processMesh(mesh, loaded_textures, shader)));
    }

    // recursively go to the next node and process those meshes too
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      processNode(meshes, node->mChildren[i], loaded_textures, shader);
    }
  }

  unsigned int ModelLoader::textureFromFile(std::string& path, const std::string& directory, bool gamma = true) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int texture_id;
    int width, height, num_components;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &num_components, 0);
    if (data)
    {
      GLenum format;
      if (num_components == 1)
        format = GL_RED;
      else if (num_components == 3)
        format = GL_RGB;
      else if (num_components == 4)
        format = GL_RGBA;

      glGenTextures(1, &texture_id);
      glBindTexture(GL_TEXTURE_2D, texture_id);
      //glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Or GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, etc.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // For the T coordinate
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glGenerateMipmap(GL_TEXTURE_2D);
      stbi_image_free(data);
    }
    else
    {
      Logger::logError("Failed to load image from ", path);
      stbi_image_free(data);
    }

    return texture_id;
  }

  std::vector<Texture> ModelLoader::loadMaterialTextures(std::vector<Texture>& textures_loaded, aiMaterial* material, aiTextureType type, std::string type_name) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
      aiString str;
      material->GetTexture(type, i, &str);
      // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
      bool skip = false;
      for (unsigned int j = 0; j < textures_loaded.size(); j++)
      {
        if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
        {
          textures.push_back(textures_loaded[j]);
          skip = true;
          break;
        }
      }
      if (!skip)
      {
        Texture texture;
        std::string st = str.C_Str();
        texture.id = textureFromFile(st, m_current_model_path);
        texture.type = type_name;
        texture.path = str.C_Str();
        textures.push_back(texture);
        textures_loaded.push_back(texture);
      }
    }
    return textures;
  }

  std::vector<Texture> ModelLoader::getTextures(const aiMesh* mesh, std::vector<Texture>& textures_loaded) {
    aiMaterial* material = m_scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture> textures;

    std::vector<Texture> diffuseMaps = loadMaterialTextures(textures_loaded, material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps = loadMaterialTextures(textures_loaded, material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    std::vector<Texture> normalMaps = loadMaterialTextures(textures_loaded, material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<Texture> heightMaps = loadMaterialTextures(textures_loaded, material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());    return textures;
  }

  bool ModelLoader::textureAlreadyLoaded(const std::string& path, const std::vector<Texture>& loaded_textures) {
    for (const auto& texture : loaded_textures)
    {
      if (texture.path == path)
      {
        return true; // Texture is already loaded
      }
    }
    return false;
  }

  std::vector<Vertex> ModelLoader::getVertices(const aiMesh* mesh) {
    std::vector<Vertex> vertices(mesh->mNumVertices);

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

        vertices[i].bitangent.x = mesh->mBitangents[i].x;
        vertices[i].bitangent.y = mesh->mBitangents[i].y;

        vertices[i].tangent.x = mesh->mTangents[i].x;
        vertices[i].tangent.y = mesh->mTangents[i].y;
      }
      else
      {
        vertices[i].texture.x = 0.0f;
        vertices[i].texture.y = 0.0f;
      }
    }
    return vertices;
  }

  std::vector<unsigned int> ModelLoader::getIndices(const aiMesh* mesh) {
    // a vertex has 3 points so a face has 3*faces amount of indices
    std::vector<unsigned int> indices(mesh->mNumFaces * 3);
    // fill indices directly
    size_t index = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace& face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++)
      {
        indices[index++] = face.mIndices[j];
      }
    }
    return indices;
  }
}
