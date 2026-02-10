#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace kogayonon_resources
{
class Mesh;
}

namespace kogayonon_utilities
{
class AssimpLoader
{
public:
  AssimpLoader();
  ~AssimpLoader();

  void createMesh( const std::string& path, kogayonon_resources::Mesh* m );

private:
  auto readFile( const std::string& path ) -> const aiScene*;
  void releaseScene();

private:
  Assimp::Importer m_importer;
};
} // namespace kogayonon_utilities