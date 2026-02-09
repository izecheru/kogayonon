#include "utilities/asset_manager/asset_manager.hpp"
#include <cgltf.h>
#include <SOIL2/SOIL2.h>
#include <assert.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include "resources/texture.hpp"
#include "resources/vertex.hpp"
using namespace kogayonon_resources;

namespace kogayonon_utilities
{

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
  destroy();
}

std::weak_ptr<Texture> AssetManager::addTextureWithoutParams( const std::string& textureName,
                                                              const std::string& texturePath )
{
  if ( m_loadedTextures.contains( texturePath ) )
  {
    if ( m_loadedTextures.at( texturePath )->getLoaded() == true )
    {
      return m_loadedTextures.at( texturePath );
    }
    else
    {
      spdlog::info( "We have the texture in the map but it is not yet loaded in OpenGl" );
    }
  }

  assert( std::filesystem::exists( texturePath ) && "Texture path does not exist" );

  unsigned int id = SOIL_load_OGL_texture( texturePath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS );

  if ( id == 0 )
  {
    spdlog::error( "SOIL failed to load texture {} ", texturePath );
    spdlog::error( "Reason{} ", SOIL_last_result() );
    return {};
  }

  // For testing, just store with 0 width/height/channels
  auto tex = std::make_shared<Texture>( id,
                                        texturePath,
                                        textureName,
                                        0, // width unknown
                                        0, // height unknown
                                        0  // channels unknown
  );

  m_loadedTextures.try_emplace( texturePath, tex );
  return m_loadedTextures.at( texturePath );
}

void AssetManager::destroy()
{
  // don't forget to join the thread
  if ( m_watchThread.joinable() )
  {
    m_watchThread.join();
  }
}

void AssetManager::init()
{
}

auto AssetManager::addTexture( const std::string& textureName ) -> Texture*
{
  return addTexture( textureName, "resources/textures/" + textureName );
}

auto AssetManager::addTexture( const std::string& textureName, const std::string& texturePath ) -> Texture*
{
  if ( m_loadedTextures.contains( texturePath ) )
  {
    if ( m_loadedTextures.at( texturePath )->getLoaded() )
    {
      return m_loadedTextures.at( texturePath ).get();
    }
    else
    {
      spdlog::info( "We have the texture in the map but it is not yet loaded in OpenGl" );
    }
  }

  assert( std::filesystem::exists( texturePath ) && "Texture path does not exist" );

  int w = 0;
  int h = 0;
  int channels = 0;
  unsigned char* data = SOIL_load_image( texturePath.c_str(), &w, &h, &channels, SOIL_LOAD_AUTO );
  if ( !data )
  {
    spdlog::error( "soil could not load data for texture {} ", texturePath );
    spdlog::error( "SOIL failed {} ", SOIL_last_result() );
    return nullptr;
  }
  auto id = SOIL_create_OGL_texture( data, &w, &h, channels, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS );

  SOIL_free_image_data( data );

  auto tex = std::make_shared<Texture>( id, texturePath, textureName, w, h, channels );
  m_loadedTextures.try_emplace( texturePath, tex );
  spdlog::info( "Loaded texture {}", textureName, texturePath );

  return m_loadedTextures.at( texturePath ).get();
}

auto AssetManager::addMesh( const std::string& meshName, const std::string& meshPath ) -> Mesh*
{
  std::lock_guard lock{ m_assetMutex };

  if ( m_loadedMeshes.contains( meshPath ) )
  {
    spdlog::info( "Mesh already loaded {} ", meshName );
    return m_loadedMeshes.at( meshPath ).get();
  }

  assert( std::filesystem::exists( meshPath ) && "mesh file does not exist" );

  cgltf_options options{};
  cgltf_data* data = nullptr;

  // Parse GLTF
  if ( cgltf_parse_file( &options, meshPath.c_str(), &data ) != cgltf_result_success )
  {
    spdlog::error( "Failed to parse GLTF {} ", meshPath );
    cgltf_free( data );
    return {};
  }

  if ( cgltf_load_buffers( &options, data, meshPath.c_str() ) != cgltf_result_success )
  {
    spdlog::error( "Failed to load GLTF buffers {} ", meshPath );
    cgltf_free( data );
    return {};
  }

  if ( cgltf_validate( data ) != cgltf_result_success )
  {
    spdlog::error( "GLTF validation failed {} ", meshPath );
    cgltf_free( data );
    return {};
  }

  std::vector<Submesh> submeshes;
  std::vector<Vertex> vertices;
  std::vector<Texture*> textures;
  std::vector<uint32_t> indices;
  std::optional<Skeleton> skeleton;

  for ( size_t i = 0; i < data->nodes_count; ++i )
  {
    auto& node = data->nodes[i];
    if ( !node.mesh )
      continue;

    cgltf_mesh& mesh = *node.mesh;
    auto transform = glm::mat4{ 1.0f };
    if ( node.has_matrix )
    {
      transform = glm::make_mat4( node.matrix );
    }
    else
    {
      auto translation =
        glm::translate( glm::mat4{ 1.0f }, glm::vec3{ node.translation[0], node.translation[1], node.translation[2] } );
      auto rotation =
        glm::mat4_cast( glm::quat{ node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2] } );
      auto scale = glm::scale( glm::mat4{ 1.0f }, glm::vec3{ node.scale[0], node.scale[1], node.scale[2] } );
      transform = translation * rotation * scale;
    }

    if ( node.skin )
    {
      cgltf_skin& cgltfSkin = *node.skin;
      skeleton = getSkeleton( data, cgltfSkin, transform );
    }

    for ( size_t j = 0; j < mesh.primitives_count; ++j )
    {
      cgltf_primitive& primitive = mesh.primitives[j];

      std::vector<glm::vec3> localPositions;
      std::vector<glm::vec3> localNormals;
      std::vector<glm::vec2> localTextureCoords;
      std::vector<uint32_t> localIndices;
      std::vector<glm::ivec4> localJointIndices;
      std::vector<glm::vec4> localWeights;
      std::vector<Vertex> localVertices;

      parseVertices(
        primitive, localPositions, localNormals, localTextureCoords, localJointIndices, localWeights, transform );

      if ( primitive.indices )
        parseIndices( primitive.indices, localIndices );

      if ( primitive.material )
        parseTextures( primitive.material, textures );

      for ( size_t x = 0; x < localPositions.size(); ++x )
      {
        localVertices.emplace_back(
          Vertex{ .translation = localPositions[x],
                  .normal = ( x < localNormals.size() ) ? localNormals[x] : glm::vec3{ 0.0f },
                  .textureCoords = ( x < localTextureCoords.size() ) ? localTextureCoords[x] : glm::vec2{ 0.0f },
                  .jointIndices = localJointIndices.empty() ? glm::ivec4{ 1 } : localJointIndices[x],
                  .weights = localWeights.empty() ? glm::vec4{ 1.0f } : localWeights[x] } );
      }

      uint32_t vertexOffset = static_cast<uint32_t>( vertices.size() );
      uint32_t indexOffset = static_cast<uint32_t>( indices.size() );

      vertices.insert( vertices.end(), localVertices.begin(), localVertices.end() );
      indices.insert( indices.end(), localIndices.begin(), localIndices.end() );

      submeshes.emplace_back( Submesh{ .vertexOffest = vertexOffset,
                                       .indexOffset = indexOffset,
                                       .indexCount = static_cast<uint32_t>( localIndices.size() ) } );
    }
  }

  auto mesh_ =
    !skeleton ? std::make_shared<Mesh>(
                  meshPath, std::move( vertices ), std::move( indices ), std::move( textures ), std::move( submeshes ) )
              : std::make_shared<Mesh>( meshPath,
                                        std::move( vertices ),
                                        std::move( indices ),
                                        std::move( textures ),
                                        std::move( submeshes ),
                                        std::move( skeleton ) );

  m_loadedMeshes.try_emplace( meshPath, mesh_ );

  cgltf_free( data );

  spdlog::info( "Loaded mesh {} ", meshName );
  return getMesh( meshPath );
}

auto AssetManager::isJointInSkin( const cgltf_skin& skin, const cgltf_node* node ) -> bool
{
  if ( !node )
    return false;

  for ( auto i = 0u; i < skin.joints_count; i++ )
  {
    if ( skin.joints[i] == node )
      return true;
  }

  return false;
};

auto AssetManager::getJointId( const cgltf_data* data, const cgltf_node* node ) -> uint32_t
{
  return static_cast<uint32_t>( node - data->nodes );
}

auto AssetManager::getInverseBindMatrices( const cgltf_data*, const cgltf_skin& skin ) -> std::vector<glm::mat4>
{
  std::vector<glm::mat4> matrices;
  if ( !skin.inverse_bind_matrices )
  {
    matrices.resize( skin.joints_count, glm::mat4{ 1.0f } );
    return matrices;
  }

  auto accessor = skin.inverse_bind_matrices;
  matrices.reserve( skin.joints_count );
  for ( auto i = 0u; i < accessor->count; i++ )
  {
    float m[16];
    cgltf_accessor_read_float( accessor, i, m, 16 );
    matrices.emplace_back( glm::make_mat4( m ) );
  }
  return matrices;
}

auto AssetManager::getGlobalPose( kogayonon_resources::Joint& bone ) -> glm::mat4
{
  auto matrix = bone.localBindPose.getMatrix();

  // if bone is root return its matrix
  if ( !bone.parent )
    return matrix;

  // recursive calculation
  // TODO serialize those matrices since they are the same unless you edit the model file
  return getGlobalPose( *bone.parent ) * matrix;
}

auto AssetManager::calculateGlobalPoses( kogayonon_resources::Skeleton& skeleton )
{
  // iterate bones
  for ( auto i = 0u; i < skeleton.joints.size(); i++ )
  {
    auto& joint = skeleton.joints.at( i );
    if ( !joint.parent )
    {
      joint.worldMatrix = joint.localBindPose.getMatrix();
    }
    else
    {
      joint.worldMatrix = joint.parent->worldMatrix * joint.localBindPose.getMatrix();
    }
    joint.offsetMatrix = joint.worldMatrix * joint.inverseBindPose;
  }
}

auto AssetManager::getJointTransform( const cgltf_node* node ) -> JointTransform
{
  auto translation = node->translation ? glm::make_vec3( node->translation ) : glm::vec3( 0.0f );
  auto rotation = node->rotation ? glm::make_quat( node->rotation ) : glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
  auto scale = node->scale ? glm::make_vec3( node->scale ) : glm::vec3( 1.0f );
  return JointTransform{ .rotation = rotation, .translation = translation, .scale = 1.0f };
}

auto AssetManager::getSkeleton( cgltf_data* data, cgltf_skin& skin, const glm::mat4& globalTransform ) -> Skeleton
{
  // const inverseBindMatrix resets the transforms to bind pose (t pose)
  //
  // localJoinMatrix is the matrix we build from joint rotation, translation, scale
  //
  // globalJointTransform is the matrix that we get by multiplying child localJointMatrix with parent's
  // localJointMatrix untill we reach root
  //
  // jointMatrix is the final product of the two matrices above, global*inverse
  Skeleton skeleton;
  skeleton.joints.resize( skin.joints_count );
  auto matrices = getInverseBindMatrices( data, skin );

  std::unordered_map<std::string, std::vector<std::string>> r;

  for ( uint32_t i = 0; i < skin.joints_count; ++i )
  {
    auto& joint = skin.joints[i];
    auto& skeletonJoint = skeleton.joints.at( i );
    skeletonJoint.id = i;
    skeletonJoint.localBindPose = getJointTransform( joint );
    skeletonJoint.inverseBindPose = matrices.at( i );
  }

  std::unordered_map<cgltf_node*, uint32_t> nodeToBone;

  // map nodes to joint ids
  for ( uint32_t i = 0; i < skin.joints_count; ++i )
  {
    if ( isJointInSkin( skin, skin.joints[i] ) )
    {
      nodeToBone[skin.joints[i]] = i;
    }
  }

  for ( auto i = 0u; i < skin.joints_count; ++i )
  {
    auto* joint = skin.joints[i];

    if ( joint->parent )
    {
      auto it = nodeToBone.find( joint->parent );
      if ( it != nodeToBone.end() )
        skeleton.joints.at( i ).parent = &skeleton.joints.at( it->second );
    }

    for ( auto j = 0u; j < joint->children_count; ++j )
    {
      auto* childNode = joint->children[j];

      auto it = nodeToBone.find( childNode );
      if ( it != nodeToBone.end() )
        skeleton.joints.at( i ).children.push_back( &skeleton.joints.at( it->second ) );
    }
  }

  // page 734 of game engine architecture
  // A global pose can be calculated by walking the hierarchy from the joint in question
  // towards the root and model-space origin, concatenating the child-to-parent (local) transforms
  // of each joint as we go.
  calculateGlobalPoses( skeleton );

  return skeleton;
}

auto AssetManager::addMesh( const std::string& meshName ) -> Mesh*
{
  return addMesh( meshName, "resources/models/" + meshName );
}

void AssetManager::uploadMeshGeometry( Mesh* mesh ) const
{
  auto& vao = mesh->getVao();
  auto& vbo = mesh->getVbo();
  auto& ebo = mesh->getEbo();

  auto& vertices = mesh->getVertices();
  auto& indices = mesh->getIndices();

  // prepare the buffers to tell OpenGL how to interpret our data
  glCreateVertexArrays( 1, &vao );
  assert( vao != 0 && "vao cannot be 0" );

  // upload data to vertex buffer
  glCreateBuffers( 1, &vbo );
  assert( vbo != 0 && "vbo cannot be 0" );

  glNamedBufferData( vbo, vertices.size() * sizeof( Vertex ), vertices.data(), GL_DYNAMIC_DRAW );

  // upload indices to element buffer
  glCreateBuffers( 1, &ebo );
  assert( ebo != 0 && "ebo cannot be 0" );
  glNamedBufferData( ebo, indices.size() * sizeof( unsigned int ), indices.data(), GL_DYNAMIC_DRAW );

  // link vao to vbo (vbo will be binded by this call)
  glVertexArrayVertexBuffer( vao, 0, vbo, 0, sizeof( Vertex ) );

  // link ebo to vao
  glVertexArrayElementBuffer( vao, ebo );

  // tell OpenGL how the data layout looks

  // position
  glEnableVertexArrayAttrib( vao, 0 );

  // normal
  glEnableVertexArrayAttrib( vao, 1 );

  // texture coordinates
  glEnableVertexArrayAttrib( vao, 2 );

  // joint indices
  glEnableVertexArrayAttrib( vao, 3 );

  // weights
  glEnableVertexArrayAttrib( vao, 4 );

  glVertexArrayAttribFormat( vao, 0, 3, GL_FLOAT, GL_FLOAT, offsetof( Vertex, translation ) );
  glVertexArrayAttribFormat( vao, 1, 3, GL_FLOAT, GL_FLOAT, offsetof( Vertex, normal ) );
  glVertexArrayAttribFormat( vao, 2, 2, GL_FLOAT, GL_FLOAT, offsetof( Vertex, textureCoords ) );
  glVertexArrayAttribIFormat( vao, 3, 4, GL_INT, offsetof( Vertex, jointIndices ) );
  glVertexArrayAttribFormat( vao, 4, 4, GL_FLOAT, GL_FLOAT, offsetof( Vertex, weights ) );

  glVertexArrayAttribBinding( vao, 0, 0 );
  glVertexArrayAttribBinding( vao, 1, 0 );
  glVertexArrayAttribBinding( vao, 2, 0 );
  glVertexArrayAttribBinding( vao, 3, 0 );
  glVertexArrayAttribBinding( vao, 4, 0 );
}

auto AssetManager::addTextureFromMemory( const std::string& textureName, const unsigned char* data )
  -> std::weak_ptr<Texture>
{
  return std::weak_ptr<Texture>();
}

auto AssetManager::getTexture( const std::string& textureName, const std::string& folder ) -> std::weak_ptr<Texture>
{
  std::filesystem::path p{ folder + textureName };
  return m_loadedTextures.at( p.string() );
}

void AssetManager::removeTexture( const std::string& path )
{
  for ( auto it = m_loadedTextures.begin(); it != m_loadedTextures.end(); ++it )
  {
    if ( it->second->getPath() == path.data() )
    {
      spdlog::info( "deleted {} ", path );
      return;
    }
  }
  spdlog::info( "file was not loaded so we did not delete anything" );
}

auto AssetManager::getTextureById( uint32_t id ) -> std::weak_ptr<Texture>
{
  for ( const auto& [texturePath, texture] : m_loadedTextures )
  {
    if ( texture->getTextureId() == id )
      return texture;
  }
  return getTexture( "default" );
}

auto AssetManager::getMesh( const std::string& meshPath ) -> Mesh*
{
  if ( !m_loadedMeshes.contains( meshPath ) )
    return nullptr;

  return m_loadedMeshes.at( meshPath ).get();
}

void AssetManager::parseVertices( cgltf_primitive& primitive,
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

void AssetManager::parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const
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

void AssetManager::parseTextures( const cgltf_material* material, std::vector<Texture*>& textures )
{
  if ( !material )
    return;

  if ( material->normal_texture.texture && material->normal_texture.texture->image )
  {
    std::string uri = material->normal_texture.texture->image->uri;
    std::filesystem::path texturePath = std::filesystem::absolute( "resources" ) / uri;
    std::string textureName = texturePath.filename().string();

    std::shared_ptr<Texture> texture;

    if ( m_loadedTextures.contains( texturePath.string() ) )
    {
      texture = m_loadedTextures.at( texturePath.string() );
    }
    else
    {
      texture = std::make_shared<Texture>( texturePath.string(), textureName );

      if ( !m_loadedTextures.contains( texturePath.string() ) )
        m_loadedTextures.emplace( texturePath.string(), texture );
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

    if ( m_loadedTextures.contains( texturePath.string() ) )
    {
      texture = m_loadedTextures.at( texturePath.string() );
    }
    else
    {
      texture = std::make_shared<Texture>( texturePath.string(), textureName );

      if ( !m_loadedTextures.contains( texturePath.string() ) )
        m_loadedTextures.emplace( texturePath.string(), texture );
    }

    textures.push_back( texture.get() );
  }
}

void AssetManager::parseAnimations( cgltf_primitive& primitive )
{
  if ( primitive.attributes )
  {
    spdlog::debug( "name prim.attr :{}", primitive.attributes[0].name );
  }
}
} // namespace kogayonon_utilities
