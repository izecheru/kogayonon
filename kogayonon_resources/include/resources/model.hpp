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
  int getAmount() const;
  void setAmount( int amount );
  void setInstanced( bool value );

  uint32_t& getInstanceBuffer();
  std::vector<glm::mat4>& getInstances();

  void addInstance( const glm::mat4 instanceM );

  inline bool& isInstanced()
  {
    return m_isIstanced;
  }

private:
  std::vector<Mesh> m_meshes;

  // this should not be here
  std::vector<glm::mat4> m_instanceMatrices;
  uint32_t m_instanceBuffer;
  int m_amount;
  bool m_isIstanced{ false };
};
} // namespace kogayonon_resources