#pragma once
#include <memory>
#include <string>
#include <vector>
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_resources
{
class Mesh
{
public:
  Mesh() = default;
  ~Mesh() = default;

  Mesh( const Mesh& other );            // copy
  Mesh& operator=( const Mesh& other ); // copy assignment

  Mesh( Mesh&& other ) noexcept = default;            // move constructor
  Mesh& operator=( Mesh&& other ) noexcept = default; // move assignment

  explicit Mesh( std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, std::vector<Texture*>&& textures );
  explicit Mesh( std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices );

  std::vector<Vertex>& getVertices();
  std::vector<uint32_t>& getIndices();
  std::vector<Texture*>& getTextures();

  uint32_t& getVao();
  uint32_t& getVbo();
  uint32_t& getEbo();

private:
  std::vector<Texture*> m_textures;
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;

  uint32_t vao;
  uint32_t vbo;
  uint32_t ebo;
};
} // namespace kogayonon_resources