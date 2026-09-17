#include "gui/directory_hierarchy.hpp"
#include "utilities/utils/utils.hpp"

DirectoryHierarchy::DirectoryHierarchy( fs::path root )
    : m_root{ build( root ) }
    , m_init{ false }
    , m_needRebuild{ false }
{
  m_init = true;
}

auto DirectoryHierarchy::getRebuildNode() -> DirectoryEntry*
{
  return m_rebuild;
}

auto DirectoryHierarchy::setRebuild( bool value ) -> void
{
  m_needRebuild = value;
}

auto DirectoryHierarchy::setNode( DirectoryEntry* entry ) -> void
{
  m_rebuild = entry;
}

auto DirectoryHierarchy::needRebuild() const -> bool
{
  return m_needRebuild;
}

auto DirectoryHierarchy::isInit() const -> bool
{
  return m_init;
}

auto DirectoryHierarchy::rebuildNode() -> void
{
  if ( !m_rebuild )
  {
    return;
  }

  DirectoryEntry node{ .path = m_rebuild->path, .open = m_rebuild->open };

  std::error_code ec{};
  fs::directory_iterator it{ node.path, fs::directory_options::skip_permission_denied, ec };

  if ( ec )
  {
    KERROR( "{}", ec.message() );
  }

  for ( const auto& entry : it )
  {
    std::error_code entryEc;
    if ( entry.is_directory( entryEc ) )
    {
      node.children.push_back( build( entry.path() ) );
    }
    else if ( entry.is_regular_file( entryEc ) )
    {
      node.files.push_back( FileEntry{ .path = entry.path() } );
    }
  }

  // set the node rebuild points to, to the one we created since this is the updated version
  *m_rebuild = node;

  // invalidate rebuild node again
  m_rebuild = nullptr;
}

auto DirectoryHierarchy::build( const fs::path& path ) -> DirectoryEntry
{
  DirectoryEntry node{};
  node.path = path;

  std::error_code ec{};
  fs::directory_iterator it{ path, fs::directory_options::skip_permission_denied, ec };

  if ( ec )
  {
    return node;
  }

  for ( const auto& entry : it )
  {
    std::error_code entryEc;
    if ( entry.is_directory( entryEc ) )
    {
      node.children.push_back( build( entry.path() ) );
    }
    else if ( entry.is_regular_file( entryEc ) )
    {
      node.files.push_back( FileEntry{ .path = entry.path() } );
    }
  }
  return node;
}
