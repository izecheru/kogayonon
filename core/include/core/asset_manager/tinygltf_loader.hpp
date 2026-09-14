#pragma once
#include "tiny_gltf_v3.h"
#include "resources/vertex.hpp"
#include "resources/mesh.hpp"
#include "resources/material.hpp"

namespace core
{
enum TextureType : uint8_t
{
  None,
  Emissive,
  Diffuse,
  Normal
};

class TinyGltfLoader
{
public:
  explicit TinyGltfLoader( std::string_view modelPath );
  ~TinyGltfLoader();

  auto processVertexData( resources::Mesh* pMesh ) -> void;
  auto parseTextureData( resources::Mesh* pMesh ) -> std::unordered_map<uint32_t, std::map<TextureType, std::string>>;

  auto imageIndexFromTexture( int textureIndex ) const -> int;

  template <typename T>
  inline auto writeAttribute( T resources::Vertex::* member,
                              const tg3_str_int_pair* attr,
                              std::vector<resources::Vertex>& localVertices ) -> void
  {
    const tg3_accessor* accessor = &m_model.accessors[attr->value];
    const tg3_buffer_view* bufferView = &m_model.buffer_views[accessor->buffer_view];
    const tg3_buffer* buffer = &m_model.buffers[bufferView->buffer];
    const size_t bufferOffset = bufferView->byte_offset + accessor->byte_offset;
    const size_t stride = bufferView->byte_stride != 0 ? bufferView->byte_stride : sizeof( T );

    if ( localVertices.size() != accessor->count )
    {
      localVertices.resize( accessor->count );
    }

    for ( uint64_t i = 0; i < accessor->count; ++i )
    {
      const size_t elementOffset = bufferOffset + i * stride;
      const float* data = reinterpret_cast<const float*>( buffer->data.data + elementOffset );
      if constexpr ( std::is_same<T, glm::vec3>() )
      {
        localVertices[i].*member = glm::vec3( data[0], data[1], data[2] );
      }
      else if constexpr ( std::is_same<T, glm::vec2>() )
      {
        localVertices[i].*member = glm::vec2( data[0], data[1] );
      }
    }
  }

private:
  tg3_parse_options m_opts{};
  tg3_error_stack m_errors{};
  tg3_model m_model{};
  std::string m_modelPath;
};
} // namespace core