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
  void ModelLoader::buildModel(std::string path, std::vector<Mesh>& meshes, std::map<std::string, Texture>& textures_loaded) {
    m_current_model_path = path.substr(0, path.find_last_of('/'));
    getScene(path);

    // we start processing mesh nodes one by one recursively
    processNode(meshes, m_scene->mRootNode, textures_loaded);
  }

  void ModelLoader::getScene(std::string& path) {
    unsigned int importOptions = aiProcess_Triangulate
      | aiProcess_OptimizeMeshes
      | aiProcess_JoinIdenticalVertices
      | aiProcess_CalcTangentSpace
      | aiProcess_FlipUVs;

    m_scene = m_importer.ReadFile(path, importOptions);
    assert(m_scene != nullptr);
  }

  Mesh ModelLoader::processMesh(const aiMesh* mesh, std::map<std::string, Texture>& textures_loaded) {
#ifdef DEBUG
    auto start = std::chrono::high_resolution_clock::now();
    Mesh& r_mesh = Mesh(std::move(getVertices(mesh)), std::move(getIndices(mesh)), std::move(getTextures(mesh, textures_loaded)));
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    Logger::logInfo("mesh process time -> ", duration.count());
#else
    Mesh& r_mesh = Mesh(std::move(getVertices(mesh)), std::move(getIndices(mesh)), getTextures(mesh, textures_loaded));
#endif
    return r_mesh;
  }

  void ModelLoader::processNode(std::vector<Mesh>& meshes, aiNode* node, std::map<std::string, Texture>& loaded_textures) {
    // get all the meshes from this current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh* mesh = m_scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(std::move(processMesh(mesh, loaded_textures)));
    }

    // recursively go to the next node and process those meshes too
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(meshes, node->mChildren[i], loaded_textures);
    }
  }

  unsigned int ModelLoader::textureFromFile(std::string& path, const std::string& directory, bool gamma = true) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int texture_id = 0;
    int width, height, num_components;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &num_components, 0);
    if (data) {
      GLenum format;
      if (num_components == 1)
        format = GL_RED;
      else if (num_components == 3)
        format = GL_RGB;
      else if (num_components == 4)
        format = GL_RGBA;
      else {
        // Fallback in case of an unexpected number of components
        format = GL_RGB;
      }

      glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
      // We allocate immutable storage for the texture
      //int levels = static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;
      glTextureStorage2D(texture_id, 1, GL_RGBA8, width, height);

      // Upload the image data to the texture
      glTextureSubImage2D(texture_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

      // Generate mipmaps
      glGenerateTextureMipmap(texture_id);

      glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT); // Or GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, etc.
      glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT); // For the T coordinate
      glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      stbi_image_free(data);
    }
    else {
      Logger::logError("Failed to load image from ", path);
      stbi_image_free(data);
    }
    return texture_id;
  }

  std::vector<Texture> ModelLoader::loadMaterialTextures(std::map<std::string, Texture>& textures_loaded, aiMaterial* material, aiTextureType type, std::string type_name) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
      aiString str;
      material->GetTexture(type, i, &str);
      std::string texture_path = str.C_Str();

      // Check if the texture is already loaded before trying to load it
      if (textureAlreadyLoaded(texture_path, textures_loaded)) {
        Logger::logError("Texture already loaded from -> ", texture_path);
        // we retrieve the already loaded texture in here
        textures.push_back(textures_loaded[texture_path]);
      }
      else {
        Texture texture; texture.id = textureFromFile(texture_path, m_current_model_path);
        texture.type = type_name;
        texture.path = texture_path;
        // TODO might need to make the mesh textures vector hold paths instead of actual texture objects
        // and just modify the render function from Model to have a texture path parameter and get it from the textures_loaded map
        textures.push_back(texture);
        textures_loaded[texture.path] = texture;
      }
    }
    return textures;
  }

  std::vector<Texture> ModelLoader::getTextures(const aiMesh* mesh, std::map<std::string, Texture>& textures_loaded) {
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

  bool ModelLoader::textureAlreadyLoaded(const std::string& path, const std::map<std::string, Texture>& loaded_textures) {
    for (const auto& texture : loaded_textures) {
      if (texture.first == path) {
        return true;
      }
    }
    return false;
  }

  std::vector<Vertex> ModelLoader::getVertices(const aiMesh* mesh) {
    std::vector<Vertex> vertices;
    vertices.resize(mesh->mNumVertices);

    // fill vertices directly
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      vertices[i].position.x = mesh->mVertices[i].x;
      vertices[i].position.y = mesh->mVertices[i].y;
      vertices[i].position.z = mesh->mVertices[i].z;

      if (mesh->HasNormals()) {
        vertices[i].normal.x = mesh->mNormals[i].x;
        vertices[i].normal.y = mesh->mNormals[i].y;
        vertices[i].normal.z = mesh->mNormals[i].z;
      }
      if (mesh->mTextureCoords[0]) {
        vertices[i].texture.x = mesh->mTextureCoords[0][i].x;
        vertices[i].texture.y = mesh->mTextureCoords[0][i].y;

        vertices[i].bitangent.x = mesh->mBitangents[i].x;
        vertices[i].bitangent.y = mesh->mBitangents[i].y;

        vertices[i].tangent.x = mesh->mTangents[i].x;
        vertices[i].tangent.y = mesh->mTangents[i].y;
      }
      else {
        vertices[i].texture.x = 0.0f;
        vertices[i].texture.y = 0.0f;
      }
    }
    return vertices;
  }

  std::vector<unsigned int> ModelLoader::getIndices(const aiMesh* mesh) {
    // a vertex has 3 points so a face has 3*faces amount of indices
    std::vector<unsigned int> indices;
    indices.resize(mesh->mNumFaces * 3);
    // fill indices directly
    size_t index = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      aiFace& face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++) {
        indices[index++] = face.mIndices[j];
      }
    }
    return indices;
  }
}
