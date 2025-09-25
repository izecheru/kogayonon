#pragma once
#include <vector>
#include "resources/mesh.hpp"

namespace kogayonon_resources
{
class Model
{
public:
  explicit Model( std::vector<Mesh>&& meshes );

  Model( const Model& other ) = default;            // copy
  Model& operator=( const Model& other ) = default; // copy assignment

  Model( Model&& other ) noexcept = default;            // move constructor
  Model& operator=( Model&& other ) noexcept = default; // move assignment

  Model() = default;

  std::vector<Mesh>& getMeshes();

private:
  std::vector<Mesh> m_meshes;
};
} // namespace kogayonon_resources