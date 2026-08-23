#include "core/asset_manager/cgltf_loader.hpp"
#include "resources/mesh.hpp"
#include "resources/skeleton.hpp"
#include "resources/texture.hpp"
#include "utilities/utils/utils.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace resources;

core::CgltfLoader::CgltfLoader()
{
}

core::CgltfLoader::~CgltfLoader()
{
}

void core::CgltfLoader::loadMesh( const std::string& path, resources::Mesh* m, std::vector<cgltf_mesh*>& meshes )
{
  auto& vertices = m->getVertices();
  auto& indices = m->getIndices();
  auto& textures = m->getTextures();
  auto& submeshes = m->getSubmeshes();

  m_data = readFile( path );

  for ( size_t i = 0; i < m_data->nodes_count; ++i )
  {
    auto& node = m_data->nodes[i];
    if ( !node.mesh )
      continue;

    // if ( node.skin )
    //{
    //   Skeleton s;
    //   s = loadSkeleton( m_data, node.skin );
    //   parseAnimations( m_data, s );
    // }

    cgltf_mesh& mesh = *node.mesh;

    meshes.push_back( node.mesh );

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

      for ( size_t x = 0; x < localPositions.size(); ++x )
      {
        float uvX{ 0.0f };
        float uvY = uvX;
        if ( j < localTextureCoords.size() )
        {
          uvX = localTextureCoords[j].x;
          uvY = localTextureCoords[j].y;
        }

        localVertices.emplace_back(
          resources::Vertex{ .translation = localPositions[j],
                             .normal = ( j < localNormals.size() ) ? localNormals[j] : glm::vec3{ 0.0f },
                             .uv = { uvX, uvY } } );
      }

      uint32_t vertexOffset = static_cast<uint32_t>( vertices.size() );
      uint32_t indexOffset = static_cast<uint32_t>( indices.size() );
      for ( auto& idx : localIndices )
      {
        idx += vertexOffset;
      }

      vertices.insert( vertices.end(), localVertices.begin(), localVertices.end() );
      indices.insert( indices.end(), localIndices.begin(), localIndices.end() );

      submeshes.emplace_back( resources::Submesh{ .vertexOffset = static_cast<uint32_t>( vertexOffset ),
                                                  .indexOffset = static_cast<uint32_t>( indexOffset ),
                                                  .indexCount = static_cast<uint32_t>( localIndices.size() ) } );
    }
  }
}

auto core::CgltfLoader::readFile( const std::string& path ) -> cgltf_data*
{
  cgltf_options options{};
  cgltf_data* data = nullptr;

  // Parse GLTF
  if ( cgltf_parse_file( &options, path.c_str(), &data ) != cgltf_result_success )
  {
    K_ERROR( "Failed to parse GLTF {} ", path );
    cgltf_free( data );
    return {};
  }

  if ( cgltf_load_buffers( &options, data, path.c_str() ) != cgltf_result_success )
  {
    K_ERROR( "Failed to load GLTF buffers {} ", path );
    cgltf_free( data );
    return {};
  }

  if ( cgltf_validate( data ) != cgltf_result_success )
  {
    K_ERROR( "GLTF validation failed {} ", path );
    cgltf_free( data );
    return {};
  }
  return data;
}

void core::CgltfLoader::parseVertices( cgltf_primitive& primitive,
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

void core::CgltfLoader::parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const
{
  indices.resize( accessor->count );
  for ( auto i = 0u; i < accessor->count; i++ )
  {
    cgltf_accessor_read_uint( accessor, i, &indices.at( i ), 1 );
  }
}

void core::CgltfLoader::parseAnimations( cgltf_data* data, Skeleton& s )
{
  std::vector<NodeAnim> anims;
  for ( auto i = 0u; i < data->animations_count; i++ )
  {
    auto& animation = data->animations[i];
    K_INFO( "animation name {}", animation.name );
    for ( auto j = 0u; j < animation.channels_count; j++ )
    {

      auto& channel = animation.channels[j];
      auto& sampler = channel.sampler;

      int jointIndex = getNodeId( channel.target_node, data->nodes, data->nodes_count );
      if ( jointIndex < 0 )
        continue;

      auto& jointAnim = s.joints[jointIndex].animationData;
      // refers to a scalar floating-point accessor with keyframe times
      auto* input = sampler->input;

      // here we store the entire timeline
      std::vector<float> timeline( input->count );
      for ( auto y = 0; y < input->count; ++y )
      {
        cgltf_accessor_read_float( input, y, &timeline.at( y ), 1 );
      }

      // output is translation, rotation, scale
      auto* output = sampler->output;

      // target node that above property animated
      auto& target = channel.target_node;

      // type of target property animated
      switch ( channel.target_path )
      {
      case cgltf_animation_path_type_translation:
        jointAnim.translation.reserve( output->count );
        for ( auto y = 0u; y < output->count; y++ )
        {
          float t[3];
          cgltf_accessor_read_float( output, y, t, 3 );
          jointAnim.translation.emplace_back( Keyframe{ timeline.at( y ), glm::vec3{ t[0], t[1], t[2] } } );
        }
        break;
      case cgltf_animation_path_type_rotation:
        // quaternion
        jointAnim.rotation.reserve( output->count );
        for ( auto y = 0u; y < output->count; y++ )
        {
          float r[4];
          cgltf_accessor_read_float( output, y, r, 4 );
          jointAnim.rotation.emplace_back( Keyframe{ timeline.at( y ), glm::quat{ r[3], r[0], r[1], r[2] } } );
        }
        break;
      case cgltf_animation_path_type_scale:
        jointAnim.scale.reserve( output->count );
        for ( auto y = 0u; y < output->count; y++ )
        {
          float s[3];
          cgltf_accessor_read_float( output, y, s, 3 );
          jointAnim.scale.emplace_back( Keyframe{ timeline.at( y ), glm::vec3{ s[0], s[1], s[2] } } );
        }
        break;
      case cgltf_animation_path_type_weights:
        K_ERROR( "weights are not implemented" );
        break;
      default:
        K_ERROR( "something went wrong" );
      }
    }
  }

#ifdef _DEBUG
  // printing the data
  K_INFO( "Printing animation data--- size {}", anims.size() );
  for ( auto& joint : s.joints )
  {
    K_INFO( "Data for node {}", joint.name );
    for ( auto& a : joint.animationData.translation )
    {
      K_INFO( "[t]time {} value {} {} {}", a.time, a.data.x, a.data.y, a.data.z );
    }

    for ( auto& a : joint.animationData.rotation )
    {
      K_INFO( "[r]time {} value {} {} {}", a.time, a.data.x, a.data.y, a.data.z, a.data.w );
    }

    for ( auto& a : joint.animationData.scale )
    {
      K_INFO( "[s]time {} value {} {} {}", a.time, a.data.x, a.data.y, a.data.z );
    }
  }
#endif
}

auto core::CgltfLoader::getLocalMatrix( const cgltf_node* node ) -> glm::mat4
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

auto core::CgltfLoader::getNodeId( cgltf_node* target, cgltf_node* allNodes, unsigned int numNodes ) -> int
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

auto core::CgltfLoader::getGlobalTransform( resources::Joint& joint ) -> glm::mat4
{
  if ( !joint.parent )
    return joint.localMatrix;

  return getGlobalTransform( *joint.parent ) * joint.localMatrix;
}

auto core::CgltfLoader::calculateGlobalMatrix( Skeleton& s )
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
void core::CgltfLoader::printChildren( Joint* joint, uint32_t level )
{
  std::string indent( level * 2, ' ' );
  K_INFO( "{}{}", indent, joint->name );

  for ( auto& child : joint->children )
  {
    printChildren( child, level + 1 );
  }
}

auto core::CgltfLoader::loadSkeleton( cgltf_data* data, cgltf_skin* skin ) -> resources::Skeleton
{
  Skeleton skeleton;
  // get all inverse bind matrices from the skin
  auto inverseBindMatrices = getInverseBindMatrices( skin );

  // the root is the first joint
  Joint root{ .parent = nullptr, .name = skin->joints[0]->name, .localMatrix = getLocalMatrix( skin->joints[0] ) };
  skeleton.joints.reserve( skin->joints_count );
  skeleton.joints.push_back( root );

  // this is used to map children and parent hierarchy
  std::unordered_map<cgltf_node*, uint32_t> nodeToIndex;

  for ( auto i = 1u; i < skin->joints_count; i++ )
  {
    auto& joint = skin->joints[i];

    Joint j{ .name = joint->name,
             .id = i,
             .gltfJointIndex = getNodeId( joint, data->nodes, data->nodes_count ),
             .inverseBind = inverseBindMatrices.at( i ) };

    skeleton.joints.push_back( j );
    nodeToIndex.emplace( joint, i );
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
  K_INFO( "__________PRINTING JOINTS_________" );
  K_INFO( "joints array size {}", skeleton.joints.size() );
  printChildren( &skeleton.joints.at( 0 ), 0 );
#endif

  return skeleton;
}

auto core::CgltfLoader::getInverseBindMatrices( cgltf_skin* skin ) -> std::vector<glm::mat4>
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

void core::CgltfLoader::freeData()
{
  if ( m_data )
    cgltf_free( m_data );
}
