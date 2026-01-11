#include "rendering/light_shader_storagebuffer.hpp"
#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace kogayonon_rendering
{
void LightShaderStoragebuffer::bind( uint32_t index )
{
  glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_ssbos.at( index ).id );
}

void LightShaderStoragebuffer::bind()
{
  for ( short i = 0; i < 3; i++ )
  {
    const auto& ssbo = m_ssbos.at( i );
    if ( ssbo.id != 0 )
      glBindBuffer( GL_SHADER_STORAGE_BUFFER, ssbo.id );
  }
}

void LightShaderStoragebuffer::unbind()
{
  glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
}

void LightShaderStoragebuffer::initialize()
{
  // 0       1       2
  //  point spot directional
  for ( uint32_t i = 0; i < 3; i++ )
  {
    m_ssbos.push_back( SSBO{ .id = 0, .bindingIndex = i } );
  }

  auto& pointLightSSBO = m_ssbos.at( 0 );
  auto& directionalLightSSBO = m_ssbos.at( 1 );
  auto& spotLightSSBO = m_ssbos.at( 2 );

  initStorageBuffer( kogayonon_resources::LightType::Point, pointLightSSBO );
  initStorageBuffer( kogayonon_resources::LightType::Directional, directionalLightSSBO );
  initStorageBuffer( kogayonon_resources::LightType::Spot, spotLightSSBO );
}

void LightShaderStoragebuffer::initStorageBuffer( const kogayonon_resources::LightType& type, SSBO& ssbo )
{
  switch ( type )
  {
  case kogayonon_resources::LightType::Point: {
    glCreateBuffers( 1, &ssbo.id );
    if ( !m_pointLights.empty() )
    {
      glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::PointLight ) * m_pointLights.size(),
                         m_pointLights.data(), GL_DYNAMIC_DRAW );
    }
    else
    {
      glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::PointLight ), nullptr, GL_DYNAMIC_DRAW );
    }
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, ssbo.bindingIndex, ssbo.id );
    break;
  }
  case kogayonon_resources::LightType::Directional: {
    glCreateBuffers( 1, &ssbo.id );
    if ( !m_directionalLights.empty() )
    {
      glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::DirectionalLight ) * m_directionalLights.size(),
                         m_directionalLights.data(), GL_DYNAMIC_DRAW );
    }
    else
    {
      glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::DirectionalLight ), nullptr, GL_DYNAMIC_DRAW );
    }
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, ssbo.bindingIndex, ssbo.id );
    break;
  }
  case kogayonon_resources::LightType::Spot: {
    glCreateBuffers( 1, &ssbo.id );
    if ( !m_spotLights.empty() )
    {
      glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::SpotLight ) * m_spotLights.size(), m_spotLights.data(),
                         GL_DYNAMIC_DRAW );
    }
    else
    {
      glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::SpotLight ), nullptr, GL_DYNAMIC_DRAW );
    }
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, ssbo.bindingIndex, ssbo.id );
    break;
  }
  }
}

void LightShaderStoragebuffer::destroy()
{
  for ( short i = 0; i < 3; i++ )
  {
    destroy( i );
  }
}

void LightShaderStoragebuffer::destroy( uint32_t index )
{
  auto& ssbo = m_ssbos.at( index );
  if ( ssbo.id != 0 )
  {
    glDeleteBuffers( 1, &ssbo.id );
    ssbo.id = 0;
  }
}

void LightShaderStoragebuffer::update()
{
  for ( short i = 0; i < 3; i++ )
  {
    update( i );
  }
}

void LightShaderStoragebuffer::update( uint32_t index )
{
  assert( index < m_ssbos.size() && "index out of range" );
  auto& ssbo = m_ssbos.at( index );
  if ( ssbo.id != 0 )
  {
    switch ( index )
    {
    case 0: {
      initStorageBuffer( kogayonon_resources::LightType::Point, ssbo );
      bind( 0 );

      if ( !m_pointLights.empty() )
        glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::PointLight ) * m_pointLights.size(),
                           m_pointLights.data(), GL_DYNAMIC_DRAW );

      glBindBufferBase( GL_SHADER_STORAGE_BUFFER, ssbo.bindingIndex, ssbo.id );
      unbind();
      break;
    }
    case 1: {
      initStorageBuffer( kogayonon_resources::LightType::Directional, ssbo );
      bind( 1 );

      if ( !m_directionalLights.empty() )
        glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::DirectionalLight ) * m_directionalLights.size(),
                           m_directionalLights.data(), GL_DYNAMIC_DRAW );

      glBindBufferBase( GL_SHADER_STORAGE_BUFFER, ssbo.bindingIndex, ssbo.id );
      unbind();
      break;
    }
    case 2: {
      initStorageBuffer( kogayonon_resources::LightType::Spot, ssbo );
      bind( 2 );

      if ( !m_spotLights.empty() )
        glNamedBufferData( ssbo.id, sizeof( kogayonon_resources::SpotLight ) * m_spotLights.size(), m_spotLights.data(),
                           GL_DYNAMIC_DRAW );

      glBindBufferBase( GL_SHADER_STORAGE_BUFFER, ssbo.bindingIndex, ssbo.id );
      unbind();
      break;
    }
    }
  }
}

auto LightShaderStoragebuffer::addLight( const kogayonon_resources::LightType& type ) -> int
{
  switch ( type )
  {
  case kogayonon_resources::LightType::Point: {
    auto light = kogayonon_resources::PointLight{};
    auto index = m_pointLights.size();
    m_pointLights.emplace_back( light );
    return index;
  }
  case kogayonon_resources::LightType::Directional: {
    auto light = kogayonon_resources::DirectionalLight{};
    auto index = m_directionalLights.size();
    m_directionalLights.emplace_back( light );
    return index;
  }
  case kogayonon_resources::LightType::Spot: {
    break;
  }
  }
}

void LightShaderStoragebuffer::removeLight( const kogayonon_resources::LightType& type, uint32_t index )
{
  switch ( type )
  {
  case kogayonon_resources::LightType::Point: {
    m_pointLights.erase( m_pointLights.begin() + index );
    break;
  }
  case kogayonon_resources::LightType::Directional: {
    m_directionalLights.erase( m_directionalLights.begin() + index );
    break;
  }
  case kogayonon_resources::LightType::Spot: {
    m_spotLights.erase( m_spotLights.begin() + index );
    break;
  }
  }
}

auto LightShaderStoragebuffer::getPointLights() -> point_lights&
{
  return m_pointLights;
}

auto LightShaderStoragebuffer::getDirectionalLights() -> directional_lights&
{
  return m_directionalLights;
}

auto LightShaderStoragebuffer::getSpotLights() -> spot_lights&
{
  return m_spotLights;
}

} // namespace kogayonon_rendering