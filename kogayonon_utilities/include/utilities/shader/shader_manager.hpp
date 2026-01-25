#pragma once
#include <unordered_map>
#include "utilities/shader/shader.hpp"

namespace kogayonon_utilities
{
class Shader;

class ShaderManager
{
public:
  ShaderManager() = default;
  ~ShaderManager() = default;

  auto getShaderId( const std::string& shaderName ) -> uint32_t;
  void pushShader( const std::string& vertexShader, const std::string& fragmentShader, const std::string& shaderName );
  auto getShader( const std::string& shaderName ) -> Shader&;
  void bindShader( const std::string& shaderName );
  void unbindShader( const std::string& shaderName );
  void removeShader( const std::string& shaderName );

  /**
   * @brief Loops through all the shaders and check for the m_isCompiled flag, if false it parses the vertex and
   * fragment files again and then compiles and links them
   */
  void compileMarkedShaders();

  /**
   * @brief Marks a shader for recompilation if either vertex or fragment shader path is == to the one in the param list
   * @param filePath Path to the file we are looking for
   */
  void markForRecompilation( const std::string& filePath );

private:
  // this mutex is not needed atm, might add if shaders take too much time to compile
  // std::mutex m_mutex;
  std::unordered_map<std::string, Shader> m_shaders;
};
} // namespace kogayonon_utilities