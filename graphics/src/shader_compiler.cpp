#include "graphics/shader_compiler.hpp"
#include "precompiled/pch.hpp"
#include "utilities/utils/utils.hpp"

graphics::ShaderCompiler::ShaderCompiler( VkDevice d )
    : m_pDevice{ d }
    , m_globalSession{ nullptr }
{
  SlangGlobalSessionDesc desc{};
  desc.structureSize = sizeof( desc );
  desc.apiVersion = SLANG_API_VERSION;
  desc.minLanguageVersion = SLANG_LANGUAGE_VERSION_2025;
  desc.enableGLSL = true;

  auto res = slang::createGlobalSession( &desc, m_globalSession.writeRef() );
  assert( res == 0 && "could not initialize shader compiler" );
}

graphics::ShaderCompiler::~ShaderCompiler()
{
}

auto graphics::ShaderCompiler::compileShaderFromSource( const std::string& shaderName,
                                                        const std::string& shaderEntryFunc ) -> ShaderObject
{
  ShaderObject obj{};
  auto slangTargets{ std::to_array<slang::TargetDesc>(
    { { .format{ SLANG_SPIRV }, .profile{ m_globalSession->findProfile( "spirv_1_6" ) } } } ) };

  auto slangOptions{ std::to_array<slang::CompilerOptionEntry>(
    { { slang::CompilerOptionName::EmitSpirvDirectly, { slang::CompilerOptionValueKind::Int, 1 } },
      { slang::CompilerOptionName::MatrixLayoutColumn, { slang::CompilerOptionValueKind::Int, 1 } },
      { slang::CompilerOptionName::MatrixLayoutRow, { slang::CompilerOptionValueKind::Int, 0 } },
      { slang::CompilerOptionName::EmitSpirvDirectly, { slang::CompilerOptionValueKind::Int, 1 } } } ) };

  const auto shaderPath = ( std::filesystem::current_path() / "engine_resources" / "shaders" ).string();

  const char* searchPaths[] = { shaderPath.c_str() };

  slang::SessionDesc slangSessionDesc{ .targets{ slangTargets.data() },
                                       .targetCount{ SlangInt( slangTargets.size() ) },
                                       .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
                                       .searchPaths = searchPaths,
                                       .searchPathCount = 1,
                                       .compilerOptionEntries{ slangOptions.data() },
                                       .compilerOptionEntryCount{ uint32_t( slangOptions.size() ) } };

  Slang::ComPtr<slang::ISession> session;

  auto res = m_globalSession->createSession( slangSessionDesc, session.writeRef() );

  if ( SLANG_FAILED( res ) )
  {
    K_ERROR( "Failed to create Slang session" );
    return obj;
  }

  Slang::ComPtr<slang::IModule> slangModule;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slangModule = session->loadModule( shaderName.c_str(), diagnosticsBlob.writeRef() );

    if ( !slangModule )
    {
      const char* diagMsg = (const char*)diagnosticsBlob->getBufferPointer();
      K_ERROR( "Failed to load module: {}", diagMsg );
      throw std::runtime_error( "Could not load slang module" );
    }
  }

  Slang::ComPtr<slang::IEntryPoint> entryPoint;
  res = slangModule->findEntryPointByName( shaderEntryFunc.c_str(), entryPoint.writeRef() );

  if ( SLANG_FAILED( res ) || !entryPoint )
  {
    K_ERROR( "Entry point not found: {}", shaderEntryFunc );
    throw std::runtime_error( "Entry point lookup failed" );
  }

  slang::IComponentType* components[] = { slangModule.get(), entryPoint.get() };
  Slang::ComPtr<slang::IComponentType> composedProgram;
  res = session->createCompositeComponentType( components, 2, composedProgram.writeRef() );

  if ( SLANG_FAILED( res ) )
  {
    throw std::runtime_error( "Could not create composite component" );
  }

  Slang::ComPtr<slang::IBlob> diagnosticsBlob;
  composedProgram->getLayout()->getEntryPointByIndex( 0 );

  res = composedProgram->getEntryPointCode( 0, 0, obj.code.writeRef(), diagnosticsBlob.writeRef() );

  if ( SLANG_FAILED( res ) )
  {
    if ( diagnosticsBlob )
    {
      const char* diagMsg = (const char*)diagnosticsBlob->getBufferPointer();
      K_ERROR( "Failed to load module: {}", diagMsg );
      throw std::runtime_error( "Failed to compile entry point to SPIR-V" );
    }
  }

  return obj;
}

auto graphics::ShaderCompiler::readFile( const std::string& filePath ) -> std::vector<char>
{
  std::ifstream file( filePath, std::ios::ate | std::ios::binary );

  if ( !file.is_open() )
  {
    throw std::runtime_error( "failed to open shader file!\n" );
  }
  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer( fileSize );
  file.seekg( 0 );
  file.read( buffer.data(), fileSize );
  file.close();

  return buffer;
}

auto graphics::ShaderCompiler::createShaderModule( const std::string& shaderName, const std::string& shaderEntryFunc )
  -> VkShaderModule
{
  auto obj = compileShaderFromSource( shaderName, shaderEntryFunc );

  VkShaderModuleCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = obj.code->getBufferSize(),
    .pCode = reinterpret_cast<const uint32_t*>( obj.code->getBufferPointer() ),
  };

  VkShaderModule shaderModule{};
  if ( ( vkCreateShaderModule( m_pDevice, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS ) )
  {
    throw std::runtime_error( "could not create shader module" );
  }
  return shaderModule;
}
