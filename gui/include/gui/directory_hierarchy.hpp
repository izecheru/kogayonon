#pragma once
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

struct FileEntry
{
  fs::path path;
};

struct DirectoryEntry
{
  fs::path path;
  std::vector<DirectoryEntry> children;
  std::vector<FileEntry> files;
  bool open{ false };
};

class DirectoryHierarchy
{
public:
  explicit DirectoryHierarchy( fs::path root );

  [[nodiscard]] auto root() noexcept -> DirectoryEntry&
  {
    return m_root;
  }

  [[nodiscard]] auto getRebuildNode() -> DirectoryEntry*;
  [[nodiscard]] auto isInit() const -> bool;
  [[nodiscard]] auto needRebuild() const -> bool;

  auto setRebuild( bool value ) -> void;
  auto setNode( DirectoryEntry* entry ) -> void;

  /**
   * @brief Build hierarchy from a single directory onwards
   * @param entry
   * @return
   */
  auto rebuildNode() -> void;

private:
  /**
   * @brief Build hierarchy from root
   * @param path
   * @return
   */
  auto build( const fs::path& path ) -> DirectoryEntry;

private:
  bool m_init;
  bool m_needRebuild;
  DirectoryEntry m_root;
  DirectoryEntry* m_rebuild;
};
