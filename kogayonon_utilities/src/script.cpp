#include "utilities/script/script.hpp"

namespace kogayonon_utilities
{
Script::Script( const std::string& path )
	: m_path{ path }
{
}

Script::Script( const std::string& path, const std::string& compilationPath )
	: m_path{ path }
	, m_compilePath{ compilationPath }
{
}

auto Script::getPath() -> std::string&
{
	return m_path;
}

auto Script::getCompilePath() -> std::string&
{
	return m_compilePath;
}
} // namespace kogayonon_utilities
