#pragma once

#include <glm/mat4x4.hpp>
#include <string>

namespace kogayonon_utilities {
enum class ShaderType
{
    NONE = 0,
    VERTEX = 1,
    FRAGMENT = 2
};

struct shader_source
{
    shader_source( const std::string& vert, const std::string& frag ) : vertex_source( vert ), fragment_source( frag )
    {}

    shader_source() = default;
    std::string vertex_source;
    std::string fragment_source;
};

class Shader
{
  public:
    Shader() = default;

    explicit Shader( const std::string& vertexPath, const std::string& fragmentPath );

    shader_source parseShaderFile( const std::string& vertexPath, const std::string& fragmentPath );

    void bind() const;
    void unbind() const;

    void setInt( const char* uniform, int value ) const;
    void setMat4( const char* uniform, glm::mat4& mat );

    unsigned int getShaderId() const;

  private:
    static unsigned int compileShader( unsigned int shaderType, std::string& sourceData );
    static int createShader( shader_source& src );

  private:
    uint32_t m_programId = 0;
    shader_source m_shaderSource;
};
} // namespace kogayonon_utilities