#include "rendering/light_shader_storagebuffer.hpp"
#include <glad/glad.h>

namespace kogayonon_rendering
{
void LightShaderStoragebuffer::bind()
{
  glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_ssbo );
}

void LightShaderStoragebuffer::unbind()
{
  glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
}

void LightShaderStoragebuffer::initialize( uint32_t bindingIndex )
{
  m_bindingIndex = bindingIndex;
  glCreateBuffers( 1, &m_ssbo );

  if ( !m_pointLights.empty() )
  {
    glNamedBufferData( m_ssbo, sizeof( kogayonon_resources::PointLight ) * m_pointLights.size(), m_pointLights.data(),
                       GL_DYNAMIC_DRAW );
  }
  else
  {
    glNamedBufferData( m_ssbo, sizeof( kogayonon_resources::PointLight ), nullptr, GL_DYNAMIC_DRAW );
  }
  glBindBufferBase( GL_SHADER_STORAGE_BUFFER, m_bindingIndex, m_ssbo );
}

void LightShaderStoragebuffer::destroy()
{
  if ( m_ssbo )
  {
    glDeleteBuffers( 1, &m_ssbo );
    m_ssbo = 0;
  }
}

void LightShaderStoragebuffer::update()
{
  if ( m_ssbo == 0 )
    initialize( m_bindingIndex );

  bind();

  if ( !m_pointLights.empty() )
    glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( kogayonon_resources::PointLight ) * m_pointLights.size(),
                  m_pointLights.data(), GL_DYNAMIC_DRAW );

  glBindBufferBase( GL_SHADER_STORAGE_BUFFER, m_bindingIndex, m_ssbo );
  unbind();
}

std::vector<kogayonon_resources::PointLight>& LightShaderStoragebuffer::getPointLights()
{
  return m_pointLights;
}

int LightShaderStoragebuffer::addPointLight()
{
  auto light = kogayonon_resources::PointLight{};
  auto index = m_pointLights.size();
  m_pointLights.emplace_back( light );
  update();
  return index;
}

} // namespace kogayonon_rendering