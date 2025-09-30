#pragma once
#include <vector>
#include "resources/mesh.hpp"

namespace kogayonon_resources
{
class Model
{
public:
  explicit Model( std::vector<Mesh>&& meshes, const std::string& path );

  Model( const Model& other ) = default;            // copy
  Model& operator=( const Model& other ) = default; // copy assignment

  Model( Model&& other ) noexcept = default;            // move constructor
  Model& operator=( Model&& other ) noexcept = default; // move assignment

  Model() = default;

  std::vector<Mesh>& getMeshes();
  std::string getPath();

private:
  std::vector<Mesh> m_meshes;
  std::string m_path;
};
} // namespace kogayonon_resources