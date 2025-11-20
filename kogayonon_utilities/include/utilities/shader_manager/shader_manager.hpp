#pragma once
#include <unordered_map>
#include "utilities/shader_manager/shader.hpp"

namespace kogayonon_utilities
{
class Shader;

class ShaderManager
{
public:
  ShaderManager() = default;
  ~ShaderManager() = default;

  unsigned int getShaderId( const std::string& shaderName );
  void pushShader( const std::string& vertexShader, const std::string& fragmentShader, const std::string& shaderName );
  Shader& getShader( const std::string& shaderName );
  void bindShader( const std::string& shaderName );
  void unbindShader( const std::string& shaderName );
  void removeShader( const std::string& shaderName );

  void compileShaders();

  /**
   * @brief Recompiles a shader if either vertex or fragment shader path is == to the one in the param list
   * @param filePath Path to the file we are looking for
   */
  void markForRecompilation( const std::string& filePath );

private:
  // this mutex is not needed atm, might add if shaders take too much time to compile
  // std::mutex m_mutex;
  std::unordered_map<std::string, Shader> m_shaders;
};
} // namespace kogayonon_utilities