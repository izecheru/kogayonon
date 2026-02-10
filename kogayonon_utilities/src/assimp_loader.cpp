#include "utilities/asset_manager/assimp_loader.hpp"
#include <assert.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>
#include "resources/mesh.hpp"

namespace kogayonon_utilities
{
AssimpLoader::AssimpLoader()
    : m_importer{}
{
}

AssimpLoader::~AssimpLoader()
{
  m_importer.FreeScene();
}

auto AssimpLoader::readFile( const std::string& path ) -> const aiScene*
{
  if ( m_importer.GetScene() )
    releaseScene();

  const aiScene* scene = m_importer.ReadFile(
    path.c_str(),
    aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
      aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure );

  if ( !scene )
  {
    spdlog::error( "Assimp error: {}", m_importer.GetErrorString() );
    return nullptr;
  }
  return scene;
}

void AssimpLoader::releaseScene()
{
  m_importer.FreeScene();
}

void AssimpLoader::createMesh( const std::string& path, kogayonon_resources::Mesh* m )
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

      std::vector<glm::vec3> localPositions;
      std::vector<glm::vec3> localNormals;
      std::vector<glm::vec2> localTextureCoords;
      std::vector<uint32_t> localIndices;
      std::vector<glm::ivec4> localJointIndices;
      std::vector<glm::vec4> localWeights;
      std::vector<kogayonon_resources::Vertex> localVertices;

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
        localVertices.emplace_back( kogayonon_resources::Vertex{
          .translation = localPositions[j],
          .normal = ( j < localNormals.size() ) ? localNormals[j] : glm::vec3{ 0.0f },
          .textureCoords = ( j < localTextureCoords.size() ) ? localTextureCoords[j] : glm::vec2{ 0.0f },
          .jointIndices = localJointIndices.empty() ? glm::ivec4{ 1 } : localJointIndices[j],
          .weights = localWeights.empty() ? glm::vec4{ 1.0f } : localWeights[j] } );
      }

      uint32_t vertexOffset = static_cast<uint32_t>( vertices.size() );
      uint32_t indexOffset = static_cast<uint32_t>( indices.size() );

      vertices.insert( vertices.end(), localVertices.begin(), localVertices.end() );
      indices.insert( indices.end(), localIndices.begin(), localIndices.end() );

      submeshes.emplace_back(
        kogayonon_resources::Submesh{ .vertexOffest = vertexOffset,
                                      .indexOffset = indexOffset,
                                      .indexCount = static_cast<uint32_t>( localIndices.size() ) } );
    }
  }
}
} // namespace kogayonon_utilities
