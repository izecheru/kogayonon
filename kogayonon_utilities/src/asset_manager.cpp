#include "utilities/asset_manager/asset_manager.hpp"
#include <cgltf.h>
#include <SOIL2/SOIL2.h>
#include <assert.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_utilities
{
AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
  // don't forget to join the thread
  if ( m_watchThread.joinable() )
  {
    m_watchThread.join();
  }
}

std::weak_ptr<kogayonon_resources::Texture> AssetManager::addTextureWithoutParams( const std::string& textureName,
                                                                                   const std::string& texturePath )
{
  if ( m_loadedTextures.contains( textureName ) )
  {
    spdlog::info( "We already have the texture {} from {} ", textureName, texturePath );
    return m_loadedTextures.at( textureName );
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
  auto tex = std::make_shared<kogayonon_resources::Texture>( id, texturePath, textureName,
                                                             0, // width unknown
                                                             0, // height unknown
                                                             0  // channels unknown
  );

  m_loadedTextures.try_emplace( textureName, tex );

  return m_loadedTextures.at( textureName );
}

kogayonon_resources::Texture* AssetManager::addTexture( const std::string& textureName )
{
  return addTexture( textureName, "resources/textures/" + textureName );
}

kogayonon_resources::Texture* AssetManager::addTexture( const std::string& textureName, const std::string& texturePath )
{
  if ( m_loadedTextures.contains( textureName ) )
  {
    spdlog::info( "We already have the texture {} from {} ", textureName, texturePath );
    return m_loadedTextures.at( textureName ).get();
  }

  // std::lock_guard lock( m_assetMutex );
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

  auto tex = std::make_shared<kogayonon_resources::Texture>( id, texturePath, textureName, w, h, channels );
  m_loadedTextures.try_emplace( textureName, tex );
  spdlog::info( "Loaded texture {} from {} ", textureName, texturePath );
  SOIL_free_image_data( data );
  return m_loadedTextures.at( textureName ).get();
}

kogayonon_resources::Mesh* AssetManager::addMesh( const std::string& meshName, const std::string& meshPath )
{
  std::lock_guard lock{ m_assetMutex };

  if ( m_loadedMeshes.contains( meshName ) )
  {
    spdlog::info( "Mesh already loaded {} ", meshName );
    return m_loadedMeshes.at( meshName ).get();
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

  std::vector<kogayonon_resources::Submesh> submeshes;
  std::vector<kogayonon_resources::Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<kogayonon_resources::Texture*> textures;

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
      std::vector<kogayonon_resources::Vertex> localVertices;

      parseVertices( primitive, localPositions, localNormals, localTextureCoords, transform );

      if ( primitive.indices )
        parseIndices( primitive.indices, localIndices );

      if ( primitive.material )
        parseTextures( primitive.material, textures );

      for ( size_t x = 0; x < localPositions.size(); ++x )
      {
        kogayonon_resources::Vertex v{ .position = localPositions[x],
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

  auto mesh_ = std::make_shared<kogayonon_resources::Mesh>( meshPath, std::move( vertices ), std::move( indices ),
                                                            std::move( textures ), std::move( submeshes ) );
  m_loadedMeshes.try_emplace( meshName, mesh_ );

  cgltf_free( data );

  spdlog::info( "Loaded mesh {} ", meshName );
  return getMesh( meshName );
}

void AssetManager::uploadMeshGeometry( kogayonon_resources::Mesh* mesh ) const
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

  glNamedBufferData( vbo, vertices.size() * sizeof( kogayonon_resources::Vertex ), vertices.data(), GL_DYNAMIC_DRAW );

  // upload indices to element buffer
  glCreateBuffers( 1, &ebo );
  assert( ebo != 0 && "ebo cannot be 0" );
  glNamedBufferData( ebo, indices.size() * sizeof( unsigned int ), indices.data(), GL_DYNAMIC_DRAW );

  // link vao to vbo (vbo will be binded by this call)
  glVertexArrayVertexBuffer( vao, 0, vbo, 0, sizeof( kogayonon_resources::Vertex ) );

  // link ebo to vao
  glVertexArrayElementBuffer( vao, ebo );

  // tell OpenGL how the data layout looks

  // position
  glEnableVertexArrayAttrib( vao, 0 );

  // normal
  glEnableVertexArrayAttrib( vao, 1 );

  // texture coordinates
  glEnableVertexArrayAttrib( vao, 2 );

  glVertexArrayAttribFormat( vao, 0, 3, GL_FLOAT, GL_FLOAT, offsetof( kogayonon_resources::Vertex, position ) );
  glVertexArrayAttribFormat( vao, 1, 3, GL_FLOAT, GL_FLOAT, offsetof( kogayonon_resources::Vertex, normal ) );
  glVertexArrayAttribFormat( vao, 2, 2, GL_FLOAT, GL_FLOAT, offsetof( kogayonon_resources::Vertex, textureCoords ) );

  glVertexArrayAttribBinding( vao, 0, 0 );
  glVertexArrayAttribBinding( vao, 1, 0 );
  glVertexArrayAttribBinding( vao, 2, 0 );
}

std::weak_ptr<kogayonon_resources::Texture> AssetManager::addTextureFromMemory( const std::string& textureName,
                                                                                const unsigned char* data )
{
  return std::weak_ptr<kogayonon_resources::Texture>();
}

std::weak_ptr<kogayonon_resources::Texture> AssetManager::getTextureByName( const std::string& textureName )
{
  auto it = m_loadedTextures.find( textureName );
  assert( it != m_loadedTextures.end() && "texture must be in the map" );
  return m_loadedTextures.at( textureName );
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

std::weak_ptr<kogayonon_resources::Texture> AssetManager::getTextureById( uint32_t id )
{
  for ( const auto& [texturePath, texture] : m_loadedTextures )
  {
    if ( texture->getTextureId() == id )
      return texture;
  }
  return getTextureByName( "default" );
}

kogayonon_resources::Mesh* kogayonon_utilities::AssetManager::getMesh( const std::string& meshName )
{
  if ( !m_loadedMeshes.contains( meshName ) )
    return nullptr;

  return m_loadedMeshes.at( meshName ).get();
}

void AssetManager::parseVertices( cgltf_primitive& primitive, std::vector<glm::vec3>& positions,
                                  std::vector<glm::vec3>& normals, std::vector<glm::vec2>& tex_coords,
                                  const glm::mat4& transformation ) const
{
  for ( size_t attr_index = 0; attr_index < primitive.attributes_count; attr_index++ )
  {
    cgltf_attribute& attribute = primitive.attributes[attr_index];
    cgltf_accessor* accessor = attribute.data;
    cgltf_buffer_view* buffer_view = accessor->buffer_view;
    assert( buffer_view != nullptr );
    uint8_t* buffer_data = (uint8_t*)buffer_view->buffer->data + buffer_view->offset;
    assert( buffer_data != nullptr );

    size_t vertex_count = accessor->count;
    size_t stride =
      buffer_view->stride ? buffer_view->stride : cgltf_calc_size( accessor->type, accessor->component_type );
    for ( size_t v = 0; v < vertex_count; v++ )
    {
      auto t_data = (float*)( buffer_data + accessor->offset + v * stride );

      switch ( attribute.type )
      {
      case cgltf_attribute_type_position:
        glm::vec3 position( t_data[0], t_data[1], t_data[2] );
        position = glm::vec3( transformation * glm::vec4( position, 1.0f ) );
        positions.push_back( position );
        break;
      case cgltf_attribute_type_normal:
        glm::vec3 normal( t_data[0], t_data[1], t_data[2] );
        normal = glm::normalize( glm::mat3( transformation ) * normal );
        normals.push_back( normal );
        break;
      case cgltf_attribute_type_texcoord:
        tex_coords.push_back( glm::vec2( t_data[0], t_data[1] ) );
        break;
      default:
        break;
      }
    }
  }
}

void AssetManager::parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices ) const
{
  cgltf_buffer_view* buffer_view = accessor->buffer_view;
  uint8_t* buffer_data = (uint8_t*)buffer_view->buffer->data + buffer_view->offset;

  for ( size_t i = 0; i < accessor->count; i++ )
  {
    uint32_t index = 0;
    if ( accessor->component_type == cgltf_component_type_r_16u )
    {
      index = *( (uint16_t*)( buffer_data + accessor->offset + i * sizeof( uint16_t ) ) );
    }
    else if ( accessor->component_type == cgltf_component_type_r_32u )
    {
      index = *( (uint32_t*)( buffer_data + accessor->offset + i * sizeof( uint32_t ) ) );
    }
    else if ( accessor->component_type == cgltf_component_type_r_8u )
    {
      index = *( (uint8_t*)( buffer_data + accessor->offset + i * sizeof( uint8_t ) ) );
    }
    indices.push_back( index );
  }
}

void AssetManager::parseTextures( const cgltf_material* material, std::vector<kogayonon_resources::Texture*>& textures )
{
  if ( !material )
    return;

  if ( material->normal_texture.texture && material->normal_texture.texture->image )
  {
    std::string uri = material->normal_texture.texture->image->uri;
    std::filesystem::path texturePath = std::filesystem::absolute( "resources" ) / uri;
    std::string textureName = texturePath.filename().string();

    std::shared_ptr<kogayonon_resources::Texture> texture;

    if ( m_loadedTextures.contains( texturePath.string() ) )
    {
      texture = m_loadedTextures.at( texturePath.string() );
    }
    else
    {
      texture = std::make_shared<kogayonon_resources::Texture>( texturePath.string(), textureName );
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

    std::shared_ptr<kogayonon_resources::Texture> texture;

    if ( m_loadedTextures.contains( texturePath.string() ) )
    {
      texture = m_loadedTextures.at( texturePath.string() );
    }
    else
    {
      texture = std::make_shared<kogayonon_resources::Texture>( texturePath.string(), textureName );
      m_loadedTextures.emplace( texturePath.string(), texture );
    }

    textures.push_back( texture.get() );
  }
}
} // namespace kogayonon_utilities