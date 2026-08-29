#pragma once
#include "precompiled/pch.hpp"

namespace utilities
{
class Script
{
public:
  explicit Script( const std::string& path );
  explicit Script( const std::string& path, const std::string& compilationPath );
  ~Script() = default;

  auto getPath() -> std::string&;
  auto getCompilePath() -> std::string&;

private:
  std::string m_path;
  std::string m_compilePath;
};
} // namespace utilities