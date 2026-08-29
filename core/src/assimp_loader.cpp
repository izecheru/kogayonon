#include "core/asset_manager/assimp_loader.hpp"
#include "precompiled/pch.hpp"
#include "resources/mesh.hpp"
#include "utilities/utils/utils.hpp"
#include <assimp/postprocess.h>
#include <assimp/vector3.h>

core::AssimpLoader::AssimpLoader()
    : m_importer{}
{
}

core::AssimpLoader::~AssimpLoader()
{
  m_importer.FreeScene();
}

const aiScene* core::AssimpLoader::readFile( const std::string& path )
{
  if ( m_importer.GetScene() )
    releaseScene();

  const aiScene* scene = m_importer.ReadFile(
    path.c_str(),
    aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
      aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure | aiProcess_FlipUVs );

  if ( !scene )
  {
    K_INFO( "Assimp error: {}", m_importer.GetErrorString() );
    return nullptr;
  }
  return scene;
}

void core::AssimpLoader::releaseScene()
{
  m_importer.FreeScene();
}

void core::AssimpLoader::loadMesh( const std::string& path, resources::Mesh* m, std::vector<aiMaterial*>& materials )
{
  std::vector<resources::Vertex>& vertices = m->getVertices();
  std::vector<uint32_t>& indices = m->getIndices();
  std::vector<resources::Submesh>& submeshes = m->getSubmeshes();
  const aiScene* scene = readFile( path );

  for ( auto i = 0u; i < scene->mNumMeshes; i++ )
  {
    aiMesh* mesh = scene->mMeshes[i];

    if ( mesh->mMaterialIndex >= 0 )
    {
      aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
      materials.push_back( material );
    }
  }

  for ( auto i = 0u; i < scene->mNumMeshes; i++ )
  {
    aiMesh* mesh = scene->mMeshes[i];

    std::vector<resources::Vertex> localVertices;
    std::vector<uint32_t> localIndices;
    std::vector<resources::Submesh> localSubmeshes;

    localVertices.reserve( mesh->mNumVertices );

    for ( unsigned j = 0; j < mesh->mNumVertices; j++ )
    {
      aiVector3D v = mesh->mVertices[j];
      aiVector3D n = mesh->mNormals[j];
      aiVector3D uv = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][j] : aiVector3D{ 0, 1, 0 };
      localVertices.emplace_back(
        resources::Vertex{ .translation = { v.x, v.y, v.z }, .normal = { n.x, n.y, n.z }, .uv = { uv.x, uv.y } } );
    }
    for ( auto j = 0u; j < mesh->mNumFaces; j++ )
    {
      aiFace& face = mesh->mFaces[j];
      for ( auto x = 0u; x < face.mNumIndices; x++ )
      {
        localIndices.emplace_back( face.mIndices[x] );
      }
    }

    uint32_t indexOffset = static_cast<uint32_t>( indices.size() );
    uint32_t vertexOffset = static_cast<uint32_t>( vertices.size() );

    localSubmeshes.emplace_back( resources::Submesh{ .vertexOffset = vertexOffset,
                                                     .indexOffset = indexOffset,
                                                     .indexCount = static_cast<uint32_t>( localIndices.size() ) } );

    vertices.insert( vertices.end(), localVertices.begin(), localVertices.end() );
    indices.insert( indices.end(), localIndices.begin(), localIndices.end() );
    submeshes.insert( submeshes.end(), localSubmeshes.begin(), localSubmeshes.end() );
  }
}
