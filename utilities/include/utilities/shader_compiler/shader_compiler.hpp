#pragma once
#include <slang-com-ptr.h>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace utilities
{
struct ShaderObject
{
  Slang::ComPtr<slang::IBlob> code;
};

class ShaderCompiler
{
public:
  explicit ShaderCompiler( VkDevice d );
  ~ShaderCompiler();

  auto readFile( const std::string& filePath ) -> std::vector<char>;
  auto createShaderModule( const std::string& shaderName ) -> VkShaderModule;

private:
  auto compileShaderFromSource( const std::string& shaderName ) -> ShaderObject;

private:
  Slang::ComPtr<slang::IGlobalSession> m_globalSession{ nullptr };
  SlangGlobalSessionDesc m_globalDesc;

  VkDevice m_pDevice;
};
} // namespace utilities