#include "utilities/asset_manager/cgltf_loader.hpp"
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include "resources/mesh.hpp"
#include "resources/texture.hpp"

using namespace kogayonon_resources;

namespace kogayonon_utilities
{

CgltfLoader::CgltfLoader()
{
}

CgltfLoader::~CgltfLoader()
{
}

void CgltfLoader::loadMesh( const std::string& path,
                            kogayonon_resources::Mesh* m,
                            std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>>& loadedTex )
{
  auto& vertices = m->getVertices();
  auto& indices = m->getIndices();
  auto& textures = m->getTextures();
  auto& submeshes = m->getSubmeshes();

  auto data = readFile( path );

  for ( size_t i = 0; i < data->nodes_count; ++i )
  {
    auto& node = data->nodes[i];
    if ( !node.mesh )
      continue;

    cgltf_mesh& mesh = *node.mesh;
    auto transform = glm::mat4( 1.0f );
    if ( node.has_matrix )
      transform = glm::make_mat4( node.matrix );
    else
    {
      glm::mat4 translation =
        glm::translate( glm::mat4{ 1.0f }, glm::vec3{ node.translation[0], node.translation[1], node.translation[2] } );
      glm::mat4 rotation =
        glm::mat4_cast( glm::quat{ node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2] } );
      glm::mat4 scale = glm::scale( glm::mat4{ 1.0f }, glm::vec3{ node.scale[0], node.scale[1], node.scale[2] } );
      transform = translation * rotation * scale;
    }

    for ( size_t j = 0; j < mesh.primitives_count; ++j )
    {
      cgltf_primitive& primitive = mesh.primitives[j];

      std::vector<glm::vec3> localPositions;
      std::vector<glm::vec3> localNormals;
      std::vector<glm::vec2> localTextureCoords;
      std::vector<uint32_t> localIndices;
      std::vector<glm::ivec4> localJointIds;
      std::vector<glm::vec4> localWeights;
      std::vector<kogayonon_resources::Vertex> localVertices;

      parseVertices(
        primitive, localPositions, localNormals, localTextureCoords, localJointIds, localWeights, transform );

      if ( primitive.indices )
        parseIndices( primitive.indices, localIndices );

      if ( primitive.material )
        parseTextures( primitive.material, textures, loadedTex );

      for ( size_t x = 0; x < localPositions.size(); ++x )
      {
        kogayonon_resources::Vertex v{ .translation = localPositions[x],
                                       .normal = ( x < localNormals.size() ) ? localNormals[x] : glm::vec3{ 0.0f },
                                       .textureCoords = ( x < localTextureCoords.size() ) ? localTextureCoords[x]
                                                                                          : glm::vec2{ 0.0f } };
        localVertices.emplace_back( v );
      }

      uint32_t vertexOffset = static_cast<uint32_t>( vertices.size() );
      uint32_t indexOffset = static_cast<uint32_t>( indices.size() );

      vertices.insert( vertices.end(), localVertices.begin(), localVertices.end() );
      indices.insert( indices.end(), localIndices.begin(), localIndices.end() );

      submeshes.emplace_back(
        kogayonon_resources::Submesh{ .vertexOffest = static_cast<uint32_t>( vertexOffset ),
                                      .indexOffset = static_cast<uint32_t>( indexOffset ),
                                      .indexCount = static_cast<uint32_t>( localIndices.size() ) } );
    }
  }

  if ( data )
    cgltf_free( data );
}

auto CgltfLoader::readFile( const std::string& path ) -> cgltf_data*
{
  cgltf_options options{};
  cgltf_data* data = nullptr;

  // Parse GLTF
  if ( cgltf_parse_file( &options, path.c_str(), &data ) != cgltf_result_success )
  {
    spdlog::error( "Failed to parse GLTF {} ", path );
    cgltf_free( data );
    return {};
  }

  if ( cgltf_load_buffers( &options, data, path.c_str() ) != cgltf_result_success )
  {
    spdlog::error( "Failed to load GLTF buffers {} ", path );
    cgltf_free( data );
    return {};
  }

  if ( cgltf_validate( data ) != cgltf_result_success )
  {
    spdlog::error( "GLTF validation failed {} ", path );
    cgltf_free( data );
    return {};
  }
  return data;
}

void CgltfLoader::parseVertices( cgltf_primitive& primitive,
                                 std::vector<glm::vec3>& positions,
                                 std::vector<glm::vec3>& normals,
                                 std::vector<glm::vec2>& tex_coords,
                                 std::vector<glm::ivec4>& jointIndices,
                                 std::vector<glm::vec4>& weights,
                                 const glm::mat4& transformation ) const
{
  for ( size_t index = 0; index < primitive.attributes_count; index++ )
  {
    cgltf_attribute& attribute = primitive.attributes[index];
    cgltf_accessor* accessor = attribute.data;
    cgltf_buffer_view* bufferView = accessor->buffer_view;
    uint8_t* bufferData = (uint8_t*)bufferView->buffer->data + bufferView->offset;

    size_t vertex_count = accessor->count;
    size_t stride =
      bufferView->stride ? bufferView->stride : cgltf_calc_size( accessor->type, accessor->component_type );
    for ( size_t v = 0; v < vertex_count; v++ )
    {
      auto t_data = (float*)( bufferData + accessor->offset + v * stride );

      switch ( attribute.type )
      {
      case cgltf_attribute_type_position:
        glm::vec3 translation( t_data[0], t_data[1], t_data[2] );
        // mesh local space, comment next line
        // translation = glm::vec3( transformation * glm::vec4( translation, 1.0f ) );
        positions.push_back( translation );
        break;
      case cgltf_attribute_type_normal:
        glm::vec3 normal( t_data[0], t_data[1], t_data[2] );
        // normal = glm::normalize( glm::mat3( transformation ) * normal );
        normals.push_back( normal );
        break;
      case cgltf_attribute_type_texcoord:
        tex_coords.push_back( glm::vec2( t_data[0], t_data[1] ) );
        break;
      case cgltf_attribute_type_joints:
        uint32_t joints_[4];
        if ( cgltf_accessor_read_uint( accessor, v, joints_, 4 ) )
        {
          jointIndices.push_back( { static_cast<int>( joints_[0] ),
                                    static_cast<int>( joints_[1] ),
                                    static_cast<int>( joints_[2] ),
                                    static_cast<int>( joints_[3] ) } );
        }
        break;
      case cgltf_attribute_type_weights:
        float weights_[4];
        if ( cgltf_accessor_read_float( accessor, v, weights_, 4 ) )
        {
          weights.push_back( { weights_[0], weights_[1], weights_[2], weights_[3] } );
        }
        break;
      default:
        break;
      }
    }
  }
}

void CgltfLoader::parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const
{
  cgltf_buffer_view* bufferView = accessor->buffer_view;
  uint8_t* bufferData = (uint8_t*)bufferView->buffer->data + bufferView->offset;

  for ( size_t i = 0; i < accessor->count; i++ )
  {
    uint32_t index = 0;
    if ( accessor->component_type == cgltf_component_type_r_16u )
    {
      index = *( (uint16_t*)( bufferData + accessor->offset + i * sizeof( uint16_t ) ) );
    }
    else if ( accessor->component_type == cgltf_component_type_r_32u )
    {
      index = *( (uint32_t*)( bufferData + accessor->offset + i * sizeof( uint32_t ) ) );
    }
    else if ( accessor->component_type == cgltf_component_type_r_8u )
    {
      index = *( (uint8_t*)( bufferData + accessor->offset + i * sizeof( uint8_t ) ) );
    }
    indices.push_back( index );
  }
}

void CgltfLoader::parseTextures(
  const cgltf_material* material,
  std::vector<Texture*>& textures,
  std::unordered_map<std::string, std::shared_ptr<kogayonon_resources::Texture>>& loadedTex )
{
  if ( !material )
    return;

  if ( material->normal_texture.texture && material->normal_texture.texture->image )
  {
    std::string uri = material->normal_texture.texture->image->uri;
    std::filesystem::path texturePath = std::filesystem::absolute( "resources" ) / uri;
    std::string textureName = texturePath.filename().string();

    std::shared_ptr<Texture> texture;

    if ( loadedTex.contains( texturePath.string() ) )
    {
      texture = loadedTex.at( texturePath.string() );
    }
    else
    {
      texture = std::make_shared<Texture>( texturePath.string(), textureName );

      if ( !loadedTex.contains( texturePath.string() ) )
        loadedTex.emplace( texturePath.string(), texture );
    }

    textures.push_back( texture.get() );
  }

  if ( material->has_pbr_metallic_roughness && material->pbr_metallic_roughness.base_color_texture.texture &&
       material->pbr_metallic_roughness.base_color_texture.texture->image )
  {
    std::string uri = material->pbr_metallic_roughness.base_color_texture.texture->image->uri;
    std::filesystem::path texturePath = std::filesystem::absolute( "resources" ) / uri;
    std::string textureName = texturePath.filename().string();

    std::shared_ptr<Texture> texture;

    if ( loadedTex.contains( texturePath.string() ) )
    {
      texture = loadedTex.at( texturePath.string() );
    }
    else
    {
      texture = std::make_shared<Texture>( texturePath.string(), textureName );

      if ( !loadedTex.contains( texturePath.string() ) )
        loadedTex.emplace( texturePath.string(), texture );
    }

    textures.push_back( texture.get() );
  }
}

void CgltfLoader::parseAnimations( cgltf_primitive& primitive )
{
  if ( primitive.attributes )
  {
    spdlog::debug( "name prim.attr :{}", primitive.attributes[0].name );
  }
}

} // namespace kogayonon_utilities
