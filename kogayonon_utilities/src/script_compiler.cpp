#include "utilities/script/script_compiler.hpp"
#include <array>
#include <memory>

namespace kogayonon_utilities
{
ScriptCompiler::ScriptCompiler()
{
	findLua55Compiler();
}

void ScriptCompiler::compileScript( Script& script )
{
}

void ScriptCompiler::pushFile( const std::string& path )
{
	for ( const auto& filePaths : m_files )
	{
		if ( filePaths != path )
		{
			m_files.emplace_back( path );
		}
	}
}

void ScriptCompiler::findLua55Compiler()
{
	// got this line from SCion2D, a very smart idea that assings _pclose as the destructor of this variable so when it
	// goes out of scope the pipe is closed, CLEAN stuff
	// https://github.com/dwjclark11/Scion2D/blob/cb9cf02b25f344de3d62ad17083b2db0f699d13c/SCION_EDITOR/src/editor/packaging/ScriptCompiler.cpp#L137
	std::unique_ptr<FILE, decltype( &_pclose )> pipe{ _popen( std::string{ "where luac55" }.c_str(), "r" ), _pclose };

	if ( !pipe )
		return;

	std::string path{};
	std::array<char, 256> buffer;

	if ( fgets( buffer.data(), buffer.size(), pipe.get() ) != nullptr )
	{
		path = buffer.data();
		path.erase( path.find_last_not_of( " \n\r" ) + 1 );
	}

	if ( path.empty() )
		return;

	m_luaCompilerPath = std::optional<std::string>{ path };
}
} // namespace kogayonon_utilities