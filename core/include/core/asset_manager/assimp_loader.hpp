#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace resources
{
class Mesh;
}

namespace core
{
class AssimpLoader
{
public:
  AssimpLoader();
  ~AssimpLoader();

  void loadMesh( const std::string& path, resources::Mesh* m, std::vector<aiMaterial*>& materials );

private:
  const aiScene* readFile( const std::string& path );
  void releaseScene();

private:
  Assimp::Importer m_importer;
};
} // namespace core