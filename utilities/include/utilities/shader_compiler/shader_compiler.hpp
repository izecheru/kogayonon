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
  ShaderCompiler();
  ~ShaderCompiler();

  auto readFile( const std::string& filePath ) -> std::vector<char>;
  auto createShaderModule( ShaderObject& obj, VkDevice device ) -> VkShaderModule;
  auto compileShaderFromSource( const std::string& shaderName ) -> ShaderObject;

private:
  Slang::ComPtr<slang::IGlobalSession> m_globalSession{ nullptr };
  SlangGlobalSessionDesc m_globalDesc;
};
} // namespace utilities