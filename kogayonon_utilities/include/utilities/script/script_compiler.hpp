#pragma once
#include <optional>
#include <string>
#include <vector>
#include "utilities/script/script.hpp"

namespace kogayonon_utilities
{

struct CompileCommand
{
	std::string outputFile;
	std::vector<std::string> scriptPaths;
};

class ScriptCompiler
{
  public:
	ScriptCompiler();
	~ScriptCompiler() = delete;

	void compileScript( Script& script );
	void pushFile( const std::string& path );

  private:
	// funcs

	/**
	 * @brief Finds the lua compiler using "where luac55"
	 */
	void findLua55Compiler();

  private:
	// m_vars
	std::vector<std::string> m_files;
	std::optional<std::string> m_luaCompilerPath{ std::nullopt };
};
} // namespace kogayonon_utilities
