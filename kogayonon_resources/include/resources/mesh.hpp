#pragma once
#include <memory>
#include <string>
#include <vector>
#include "resources/vertex.hpp"

namespace kogayonon_resources
{
struct MeshGPU
{
  unsigned int vao{ 0 };
  unsigned int vbo{ 0 };
  unsigned int ebo{ 0 };
};

class Mesh
{
public:
  Mesh() = default;
  ~Mesh() = default;

  Mesh( const Mesh& other ) = default;            // copy
  Mesh& operator=( const Mesh& other ) = default; // copy assignment

  Mesh( Mesh&& other ) noexcept = default;            // move constructor
  Mesh& operator=( Mesh&& other ) noexcept = default; // move assignment

  explicit Mesh( std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices,
                 std::vector<unsigned int>&& textures );
  explicit Mesh( std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices );

  std::vector<Vertex>& getVertices();
  std::vector<unsigned int>& getIndices();
  std::vector<unsigned int>& getTextures();

private:
  std::vector<unsigned int> m_textures;
  std::vector<Vertex> m_vertices;
  std::vector<unsigned int> m_indices;

  std::unique_ptr<MeshGPU> m_gpu;

  bool m_init = false;
};
} // namespace kogayonon_resources