#include "utilities/asset_manager/cgltf_loader.hpp"
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include "resources/mesh.hpp"
#include "resources/skeleton.hpp"
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

  parseAnimations( data );

  for ( size_t i = 0; i < data->nodes_count; ++i )
  {
    auto& node = data->nodes[i];
    if ( !node.mesh )
      continue;

    Skeleton s;
    if ( node.skin )
    {
      s = loadSkeleton( data, node.skin );
    }

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
      std::vector<Vertex> localVertices;

      parseVertices(
        primitive, localPositions, localNormals, localTextureCoords, localJointIds, localWeights, transform );

      if ( primitive.indices )
        parseIndices( primitive.indices, localIndices );

      if ( primitive.material )
        parseTextures( primitive.material, textures, loadedTex );

      for ( size_t x = 0; x < localPositions.size(); ++x )
      {
        localVertices.emplace_back( kogayonon_resources::Vertex{
          .translation = localPositions[x],
          .normal = ( x < localNormals.size() ) ? localNormals[x] : glm::vec3{ 0.0f },
          .textureCoords = ( x < localTextureCoords.size() ) ? localTextureCoords[x] : glm::vec2{ 0.0f } } );
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
  for ( auto i = 0; i < primitive.attributes_count; i++ )
  {
    cgltf_attribute& attribute = primitive.attributes[i];
    cgltf_accessor* accessor = attribute.data;
    cgltf_buffer_view* bufferView = accessor->buffer_view;
    uint8_t* bufferData = (uint8_t*)bufferView->buffer->data + bufferView->offset;

    size_t vertex_count = accessor->count;
    size_t stride =
      bufferView->stride ? bufferView->stride : cgltf_calc_size( accessor->type, accessor->component_type );
    for ( auto index = 0u; index < vertex_count; index++ )
    {
      auto t_data = (float*)( bufferData + accessor->offset + index * stride );

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
        if ( cgltf_accessor_read_uint( accessor, index, joints_, 4 ) )
        {
          jointIndices.push_back( { static_cast<int>( joints_[0] ),
                                    static_cast<int>( joints_[1] ),
                                    static_cast<int>( joints_[2] ),
                                    static_cast<int>( joints_[3] ) } );
        }
        break;
      case cgltf_attribute_type_weights:
        float weights_[4];
        if ( cgltf_accessor_read_float( accessor, index, weights_, 4 ) )
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

void CgltfLoader::parseAnimations( cgltf_data* data )
{
  for ( auto i = 0u; i < data->animations_count; i++ )
  {
    auto& animation = data->animations[i];
    spdlog::info( "animation name {}", animation.name );
    for ( auto j = 0u; j < animation.channels_count; j++ )
    {
      auto& channel = animation.channels[j];
      auto& sampler = channel.sampler;

      // refers to a scalar floating-point accessor with keyframe times
      auto* input = sampler->input;

      for ( auto y = 0; y < input->count; ++y )
      {
        float result[1];
        if ( cgltf_accessor_read_float( input, y, result, 1 ) )
        {
          // times are increasing so i got this right
          spdlog::info( "{}", result[0] );
        }
      }
      spdlog::info( "--------" );

      // output is translation, rotation, scale
      auto* output = sampler->output;

      // target node that above property animated
      auto& target = channel.target_node;

      // type of target property animated
      switch ( channel.target_path )
      {
      case cgltf_animation_path_type_translation:
        float t[3];
        cgltf_accessor_read_float( output, 0, t, 3 );
        break;
      case cgltf_animation_path_type_rotation:
        // quaternion
        float r[4];
        cgltf_accessor_read_float( output, 0, r, 4 );
        break;
      case cgltf_animation_path_type_scale:
        float s[3];
        cgltf_accessor_read_float( output, 0, s, 3 );
        break;
      case cgltf_animation_path_type_weights:
        spdlog::error( "weights are not implemented" );
        break;
      default:
        spdlog::error( "something went wrong" );
      }
    }
  }
}

auto CgltfLoader::getLocalMatrix( const cgltf_node* node ) -> glm::mat4
{
  if ( node->has_matrix )
    return glm::mat4{ *node->matrix };

  glm::vec3 translation = node->has_translation ? glm::vec3{ *node->translation } : glm::vec3{ 0.0f };
  glm::vec3 scale = node->has_scale ? glm::vec3{ *node->scale } : glm::vec3{ 1.0f };
  glm::quat rotation = node->has_rotation
                         ? glm::quat( node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2] )
                         : glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );

  return glm::translate( glm::mat4{ 1.0f }, translation ) * glm::mat4_cast( rotation ) *
         glm::scale( glm::mat4{ 1.0f }, scale );
}

auto CgltfLoader::getNodeId( cgltf_node* target, cgltf_node* allNodes, unsigned int numNodes ) -> int
{
  if ( target == 0 )
  {
    return -1;
  }
  for ( unsigned int i = 0; i < numNodes; ++i )
  {
    if ( target == &allNodes[i] )
    {
      return (int)i;
    }
  }
  return -1;
}

auto CgltfLoader::getGlobalTransform( kogayonon_resources::Joint& joint ) -> glm::mat4
{
  if ( !joint.parent )
    return joint.localMatrix;

  return getGlobalTransform( *joint.parent ) * joint.localMatrix;
}

auto CgltfLoader::calculateGlobalMatrix( Skeleton& s )
{
  for ( auto& joint : s.joints )
  {
    joint.globalMatrix = getGlobalTransform( joint );
  }
}

/**
 * @brief Recursively print the children of nodes to see if the hierachy coincides with the one from blender
 * @param joint The actual joint we are currently at
 * @param level The number of indentations
 */
void CgltfLoader::printChildren( Joint* joint, uint32_t level )
{
  std::string indent( level * 2, ' ' );
  spdlog::info( "{}{}", indent, joint->name );

  for ( auto& child : joint->children )
  {
    printChildren( child, level + 1 );
  }
}

auto CgltfLoader::loadSkeleton( cgltf_data* data, cgltf_skin* skin ) -> kogayonon_resources::Skeleton
{
  Skeleton skeleton;
  // get all inverse bind matrices from the skin
  auto inverseBindMatrices = getInverseBindMatrices( skin );

  // the root is the first joint
  Joint root{ .parent = nullptr, .name = skin->joints[0]->name, .localMatrix = getLocalMatrix( skin->joints[0] ) };
  skeleton.joints.reserve( skin->joints_count );
  skeleton.gltfNodeIndex.resize( skin->joints_count );
  skeleton.joints.push_back( root );

  std::unordered_map<cgltf_node*, uint32_t> nodeToIndex;
  for ( auto i = 1u; i < skin->joints_count; i++ )
  {
    auto& joint = skin->joints[i];
    Joint j{ .id = i, .name = joint->name, .inverseBind = inverseBindMatrices.at( i ) };
    skeleton.joints.push_back( j );
    nodeToIndex.emplace( joint, i );
    skeleton.gltfNodeIndex.push_back( getNodeId( joint, data->nodes, data->nodes_count ) );
  }

  // setup hierarchy
  for ( auto i = 0u; i < skin->joints_count; i++ )
  {
    auto& joint = skin->joints[i];
    if ( joint->parent )
    {
      if ( nodeToIndex.find( joint->parent ) != nodeToIndex.end() )
      {
        skeleton.joints.at( i ).parent = &skeleton.joints.at( nodeToIndex.at( joint->parent ) );
      }

      // allocate space for children vector
      skeleton.joints.at( i ).children.reserve( joint->children_count );
      for ( auto j = 0u; j < joint->children_count; j++ )
      {
        auto& child = joint->children[j];
        if ( nodeToIndex.find( child ) != nodeToIndex.end() )
        {
          skeleton.joints.at( i ).children.emplace_back( &skeleton.joints.at( nodeToIndex.at( child ) ) );
        }
      }
    }
  }

#ifdef _DEBUG
  spdlog::info( "__________PRINTING JOINTS_________" );
  spdlog::info( "joints array size {}", skeleton.joints.size() );
  printChildren( &skeleton.joints.at( 0 ), 0 );
#endif

  return skeleton;
}

auto CgltfLoader::getInverseBindMatrices( cgltf_skin* skin ) -> std::vector<glm::mat4>
{
  std::vector<glm::mat4> inverse;
  // get the accessor
  auto* inverseBindAccessor = skin->inverse_bind_matrices;

  // reserve enough space
  inverse.reserve( inverseBindAccessor->count );

  // iterate and read each matrix
  for ( auto i = 0u; i < inverseBindAccessor->count; i++ )
  {
    float matrix[16];
    cgltf_accessor_read_float( inverseBindAccessor, i, matrix, 16 );

    // add the matrix to the vector
    inverse.emplace_back( glm::mat4{ *matrix } );
  }
  return inverse;
}
} // namespace kogayonon_utilities
