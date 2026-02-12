#include "utilities/asset_manager/asset_manager.hpp"
#include <cgltf.h>
#include <SOIL2/SOIL2.h>
#include <assert.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include "resources/texture.hpp"
#include "resources/vertex.hpp"
#include "utilities/asset_manager/assimp_loader.hpp"

using namespace kogayonon_resources;

namespace kogayonon_utilities
{

AssetManager::AssetManager()
    : m_assimpLoader{ std::make_shared<AssimpLoader>() }
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

auto AssetManager::loadCgltfFile( const std::string& path ) -> cgltf_data*
{
  cgltf_options options{};
  cgltf_data* data = nullptr;

  // Parse GLTF
  if ( cgltf_parse_file( &options, path.c_str(), &data ) != cgltf_result_success )
  {
    spdlog::error( "Failed to parse GLTF {} ", path );
    cgltf_free( data );
    return nullptr;
  }

  if ( cgltf_load_buffers( &options, data, path.c_str() ) != cgltf_result_success )
  {
    spdlog::error( "Failed to load GLTF buffers {} ", path );
    cgltf_free( data );
    return nullptr;
  }

  if ( cgltf_validate( data ) != cgltf_result_success )
  {
    spdlog::error( "GLTF validation failed {} ", path );
    cgltf_free( data );
    return nullptr;
  }
  return data;
}

void AssetManager::freeCgltf( cgltf_data* data )
{
  if ( data )
  {
    cgltf_free( data );
  }
}

auto AssetManager::addMesh( const std::string& meshName, const std::string& meshPath ) -> Mesh*
{
  std::lock_guard lock{ m_assetMutex };

  if ( m_loadedMeshes.contains( meshPath ) )
    return m_loadedMeshes.at( meshPath ).get();

  assert( std::filesystem::exists( meshPath ) && "mesh file does not exist" );
  auto mesh = std::make_shared<Mesh>();

  mesh->getPath() = meshPath;
  std::filesystem::path p{ meshPath };
  // if we dont have gltf use assimp
  if ( p.extension().string().find( "gltf" ) == std::string::npos )
  {
    m_assimpLoader->loadMesh( meshPath, mesh.get() );
  }
  else // otherwise use cgltf
  {
    m_cgltfLoader->loadMesh( meshPath, mesh.get(), m_loadedTextures );
  }

  m_loadedMeshes.try_emplace( meshPath, mesh );

  spdlog::info( "Loaded mesh {} ", meshName );
  return getMesh( meshPath );
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

} // namespace kogayonon_utilities
