#pragma once
#include <memory>
#include <string>
#include <vector>

namespace kogayonon_resources
{
struct Vertex;
class MeshData;

struct MeshGPU
{
  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;
};

class Mesh
{
public:
  Mesh() = default;
  ~Mesh() = default;
  Mesh( const Mesh& other );
  Mesh& operator=( const Mesh& other );
  Mesh& operator=( Mesh&& other ) noexcept;

  explicit Mesh( std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                 std::vector<unsigned int>& textures );
  explicit Mesh( std::vector<Vertex>& vertices, std::vector<unsigned int>& indices );

  std::vector<Vertex>& getVertices();
  std::vector<unsigned int>& getIndices();
  std::vector<unsigned int>& getTextures();

private:
  std::shared_ptr<MeshData> m_data;
  std::shared_ptr<MeshGPU> m_gpu;

  bool m_init = false;
};
} // namespace kogayonon_resources