#pragma once

#include <filesystem>
#include <glm/mat4x4.hpp>
#include <string>

namespace kogayonon_utilities
{
enum class ShaderType
{
  NONE = 0,
  VERTEX = 1,
  FRAGMENT = 2
};

struct shader_source
{
  std::string vertexSource;
  std::string fragmentSource;
  std::filesystem::path vertexPath;
  std::filesystem::path fragmentPath;
};

class Shader
{
public:
  Shader() = default;

  shader_source parseShaderFile( const std::string& vertexPath, const std::string& fragmentPath );

  void bind() const;
  void unbind() const;

  void setInt( const char* uniform, int value ) const;
  void setMat4( const char* uniform, const glm::mat4& mat );
  void setBool( const char* uniform, bool value ) const;

  void initializeShaderSource( const std::string& vertexPath, const std::string& fragmentPath );
  void destroy() const;

  void markForCompilation();
  bool isCompiled() const;

  void initializeProgram();

  auto getVertexShaderPath() -> std::string;
  auto getFragmentShaderPath() -> std::string;

  auto getShaderId() const -> uint32_t;

private:
  auto compileShader( uint32_t shaderType, std::string& sourceData ) -> uint32_t;
  auto createShader() -> uint32_t;

private:
  uint32_t m_programId = 0;
  bool m_isCompiled{ false };
  shader_source m_shaderSource;
};
} // namespace kogayonon_utilities