#include "core/asset_manager/assimp_loader.hpp"
#include "resources/mesh.hpp"
#include "utilities/utils/utils.hpp"
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

core::AssimpLoader::AssimpLoader()
    : m_importer{}
{
}

core::AssimpLoader::~AssimpLoader()
{
  m_importer.FreeScene();
}

auto core::AssimpLoader::readFile( const std::string& path ) -> const aiScene*
{
  if ( m_importer.GetScene() )
    releaseScene();

  const aiScene* scene = m_importer.ReadFile(
    path.c_str(),
    aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
      aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure | aiProcess_FlipUVs );

  if ( !scene )
  {
    spdlog::error( "Assimp error: {}", m_importer.GetErrorString() );
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
  auto& vertices = m->getVertices();
  auto& indices = m->getIndices();
  auto& submeshes = m->getSubmeshes();
  auto scene = readFile( path );

  if ( scene->HasMeshes() )
  {
    for ( auto i = 0u; i < scene->mNumMeshes; i++ )
    {
      auto& mesh = scene->mMeshes[i];

      // check for textures
      if ( mesh->mMaterialIndex >= 0 )
      {
        auto material = scene->mMaterials[mesh->mMaterialIndex];
        materials.push_back( material );
      }
    }

    for ( auto i = 0u; i < scene->mNumMeshes; i++ )
    {
      auto& mesh = scene->mMeshes[i];

      std::vector<glm::vec3> localPositions;
      std::vector<glm::vec3> localNormals;
      std::vector<glm::vec2> localTextureCoords;
      std::vector<uint32_t> localIndices;
      std::vector<resources::Submesh> localSubmeshes;
      // std::vector<glm::ivec4> localJointIndices;
      // std::vector<glm::vec4> localWeights;
      std::vector<resources::Vertex> localVertices;

      for ( unsigned j = 0; j < mesh->mNumVertices; j++ )
      {
        aiVector3D v = mesh->mVertices[j];
        localPositions.emplace_back( v.x, v.y, v.z );

        if ( mesh->HasNormals() )
        {
          aiVector3D n = mesh->mNormals[j];
          localNormals.emplace_back( n.x, n.y, n.z );
        }
        else
        {
          localNormals.emplace_back( 0.0f );
        }

        if ( mesh->HasTextureCoords( 0 ) )
        {
          aiVector3D uv = mesh->mTextureCoords[0][j];
          localTextureCoords.emplace_back( uv.x, uv.y );
        }
        else
        {
          localTextureCoords.emplace_back( 0.0f );
        }
      }
      for ( auto j = 0u; j < mesh->mNumFaces; j++ )
      {
        auto& face = mesh->mFaces[j];
        for ( auto x = 0u; x < face.mNumIndices; x++ )
        {
          localIndices.emplace_back( face.mIndices[x] );
        }
      }

      for ( auto j = 0u; j < localPositions.size(); j++ )
      {
        localVertices.emplace_back(
          resources::Vertex{ .translation = localPositions[j],
                             .normal = ( j < localNormals.size() ) ? localNormals[j] : glm::vec3{ 0.0f },
                             .uv = { localTextureCoords[j].x, localTextureCoords[j].y } } );
      }
      uint32_t indexOffset = static_cast<uint32_t>( indices.size() );
      uint32_t vertexOffset = static_cast<uint32_t>( vertices.size() );

      localSubmeshes.emplace_back( resources::Submesh{ .vertexOffset = vertexOffset,
                                                       .indexOffset = indexOffset,
                                                       .indexCount = static_cast<uint32_t>( localIndices.size() ) } );

      for ( auto& idx : localIndices )
      {
        idx += vertexOffset;
      }

      vertices.insert( vertices.end(), localVertices.begin(), localVertices.end() );
      indices.insert( indices.end(), localIndices.begin(), localIndices.end() );
      submeshes.insert( submeshes.end(), localSubmeshes.begin(), localSubmeshes.end() );
    }
  }
}
