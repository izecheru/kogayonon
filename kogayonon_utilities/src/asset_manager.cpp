#include "utilities/asset_manager/asset_manager.hpp"
#include <cgltf.h>
#include <SOIL2/SOIL2.h>
#include <assert.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include "resources/model.hpp"
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
  if ( const auto& it = m_loadedTextures.find( textureName ); it != m_loadedTextures.end() )
  {
    spdlog::info( "We already have the texture {} from {} ", textureName, texturePath );
    return it->second;
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

  m_loadedTextures.emplace( textureName, tex );
  spdlog::info( "Loaded texture {} {}", textureName, texturePath );

  return m_loadedTextures.at( textureName );
}

std::weak_ptr<kogayonon_resources::Texture> AssetManager::addTexture( const std::string& textureName,
                                                                      const std::string& texturePath )
{
  if ( const auto& it = m_loadedTextures.find( textureName ); it != m_loadedTextures.end() )
  {
    spdlog::info( "We already have the texture {} from {} ", textureName, texturePath );
    return it->second;
  }

  // std::lock_guard lock( m_assetMutex );
  assert( std::filesystem::exists( texturePath ) && "Texture path does not exist" );

  int w = 0, h = 0, channels = 0;
  unsigned char* data = SOIL_load_image( texturePath.c_str(), &w, &h, &channels, SOIL_LOAD_AUTO );
  if ( !data )
  {
    spdlog::error( "soil could not load data for texture {} ", texturePath );
    spdlog::error( "SOIL failed {} ", SOIL_last_result() );
    return std::weak_ptr<kogayonon_resources::Texture>();
  }
  auto id = SOIL_create_OGL_texture( data, &w, &h, channels, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS );

  auto tex = std::make_shared<kogayonon_resources::Texture>( id, texturePath, textureName, w, h, channels );
  m_loadedTextures.emplace( textureName, std::move( tex ) );
  spdlog::info( "Loaded texture {} from {} ", textureName, texturePath );
  SOIL_free_image_data( data );
  return m_loadedTextures.at( textureName );
}

std::weak_ptr<kogayonon_resources::Model> AssetManager::addModel( const std::string& modelName,
                                                                  const std::string& modelPath )
{
  // std::lock_guard lock( m_assetMutex );

  if ( auto it = m_loadedModels.find( modelName ); it != m_loadedModels.end() )
  {
    spdlog::info( "Model already loaded {} ", modelName );
    return it->second;
  }

  if ( !std::filesystem::exists( modelPath ) )
  {
    spdlog::error( "Model file does not exist {} ", modelPath );
    return {};
  }

  cgltf_options options{};
  cgltf_data* data = nullptr;

  // Parse GLTF
  if ( cgltf_parse_file( &options, modelPath.c_str(), &data ) != cgltf_result_success )
  {
    spdlog::error( "Failed to parse GLTF {} ", modelPath );
    return {};
  }

  if ( cgltf_load_buffers( &options, data, modelPath.c_str() ) != cgltf_result_success )
  {
    spdlog::error( "Failed to load GLTF buffers {} ", modelPath );
    cgltf_free( data );
    return {};
  }

  if ( cgltf_validate( data ) != cgltf_result_success )
  {
    spdlog::error( "GLTF validation failed {} ", modelPath );
    cgltf_free( data );
    return {};
  }

  std::vector<kogayonon_resources::Mesh> meshes;

  for ( size_t i = 0; i < data->nodes_count; ++i )
  {
    auto& node = data->nodes[i];
    if ( !node.mesh )
      continue;

    cgltf_mesh& mesh = *node.mesh;

    glm::mat4 transform = glm::mat4( 1.0f );
    if ( node.has_matrix )
      transform = glm::make_mat4( node.matrix );
    else
    {
      glm::mat4 translation =
        glm::translate( glm::mat4( 1.0f ), glm::vec3( node.translation[0], node.translation[1], node.translation[2] ) );
      glm::mat4 rotation =
        glm::mat4_cast( glm::quat( node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2] ) );
      glm::mat4 scale = glm::scale( glm::mat4( 1.0f ), glm::vec3( node.scale[0], node.scale[1], node.scale[2] ) );
      transform = translation * rotation * scale;
    }

    for ( size_t j = 0; j < mesh.primitives_count; ++j )
    {
      cgltf_primitive& primitive = mesh.primitives[j];

      std::vector<glm::vec3> positions;
      std::vector<glm::vec3> normals;
      std::vector<glm::vec2> texCoords;
      std::vector<unsigned int> textureIDs;
      std::vector<unsigned int> indices;

      parseVertices( primitive, positions, normals, texCoords, transform );

      if ( primitive.indices )
        parseIndices( primitive.indices, indices );

      if ( primitive.material )
        parseTextures( primitive.material, textureIDs );

      std::vector<kogayonon_resources::Vertex> vertices;
      for ( size_t x = 0; x < positions.size(); ++x )
      {
        kogayonon_resources::Vertex v;
        v.position = positions[x];
        v.normal = ( x < normals.size() ) ? normals[x] : glm::vec3( 0.0f );
        v.textureCoords = ( x < texCoords.size() ) ? texCoords[x] : glm::vec2( 0.0f );
        vertices.push_back( v );
      }

      meshes.emplace_back( std::move( vertices ), std::move( indices ), std::move( textureIDs ) );
    }
  }

  auto model = std::make_shared<kogayonon_resources::Model>( std::move( meshes ) );

  m_loadedModels.emplace( modelName, std::move( model ) );

  cgltf_free( data );

  spdlog::info( "Loaded model {} ", modelName );
  return getModel( modelName );
}

std::weak_ptr<kogayonon_resources::Texture> AssetManager::addTextureFromMemory( const std::string& textureName,
                                                                                const unsigned char* data )
{
  return std::weak_ptr<kogayonon_resources::Texture>();
}

std::weak_ptr<kogayonon_resources::Texture> AssetManager::getTexture( const std::string& textureName )
{
  auto it = m_loadedTextures.find( textureName );
  assert( it != m_loadedTextures.end() && "texture must be in the map" );
  return m_loadedTextures.at( textureName );
}

void AssetManager::removeTexture( const std::string& path )
{
  for ( auto it = m_loadedTextures.begin(); it != m_loadedTextures.end(); ++it )
  {
    if ( it->second->getPath() == path )
    {
      spdlog::info( "deleted {} ", path );
      return;
    }
  }
  spdlog::info( "file was not loaded so we did not delete anything" );
}

std::weak_ptr<kogayonon_resources::Model> kogayonon_utilities::AssetManager::getModel( const std::string& modelName )
{
  auto it = m_loadedModels.find( modelName );
  assert( it != m_loadedModels.end() && "model must be in the map" );
  return m_loadedModels.at( modelName );
}

void AssetManager::parseVertices( cgltf_primitive& primitive, std::vector<glm::vec3>& positions,
                                  std::vector<glm::vec3>& normals, std::vector<glm::vec2>& tex_coords,
                                  const glm::mat4& transformation )
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

void AssetManager::parseIndices( cgltf_accessor* accessor, std::vector<uint32_t>& indices )
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

void AssetManager::parseTextures( const cgltf_material* material, std::vector<unsigned int>& textureIDs )
{
  if ( !material )
    return;

  if ( material->normal_texture.texture && material->normal_texture.texture->image )
  {
    std::string uri = material->normal_texture.texture->image->uri;
    std::filesystem::path texturePath( "resources/" + uri );
    std::string textureName = texturePath.filename().string();

    auto pTexture = addTexture( textureName, texturePath.string() );
    if ( auto tex = pTexture.lock() )
      textureIDs.push_back( tex->getTextureId() );
  }

  if ( material->has_pbr_metallic_roughness && material->pbr_metallic_roughness.base_color_texture.texture &&
       material->pbr_metallic_roughness.base_color_texture.texture->image )
  {
    std::string uri = material->pbr_metallic_roughness.base_color_texture.texture->image->uri;

    // since the model is exported with textures it will have textures/texture.png as it's uri
    // but we placed the textures dir in the resources one so we must have resources/textures/texture.png
    std::filesystem::path texturePath = std::filesystem::path( "resources" ) / uri;
    std::string textureName = texturePath.filename().string();

    auto pTexture = addTexture( textureName, texturePath.string() );
    if ( auto tex = pTexture.lock() )
      textureIDs.push_back( tex->getTextureId() );
  }
}
} // namespace kogayonon_utilities