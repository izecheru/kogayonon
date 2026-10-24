#include "core/input/key_map_manager.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/event/event_dispatcher.hpp"

core::KeyMapManager::KeyMapManager( const std::filesystem::path& mapPath, EventDispatcher* dispatcher )
    : m_mapPath{ mapPath }
{
    dispatcher->addHandler<core::KeyPressedEvent, &KeyMapManager::onKeyPressed>( *this );
}

core::KeyMapManager::KeyMapManager( EventDispatcher* dispatcher )
    : KeyMapManager{ std::filesystem::current_path(), dispatcher }
{
}

auto core::KeyMapManager::updateGroup( const std::string& actionGroup ) -> void
{
    if ( !m_binds.contains( actionGroup ) || !m_actions.contains( actionGroup ) )
        throw std::runtime_error( "You did not register the action group" );

    for ( auto& [key, action] : m_binds[actionGroup] )
    {
        if ( KeyboardState::getKeyState( key ) )
        {
            auto& func = m_actions[actionGroup][action];
            func();
        }
    }
}

auto core::KeyMapManager::registerAction( const std::string& actionGroup, EAction action, std::function<void()> func )
    -> void
{
    if ( m_actions[actionGroup].contains( action ) )
    {
        std::string message{ "Action group " + actionGroup + " already contains the action!!!" };
        throw std::runtime_error( message );
    }

    m_actions[actionGroup].emplace( action, std::move( func ) );
}

auto core::KeyMapManager::bindAction( const std::string& actionGroup, utilities::KeyScanCode key, EAction action )
    -> void
{
    if ( m_binds[actionGroup].contains( key ) )
    {
        std::string message{ "Action bind group " + actionGroup + " already contains the key!!!" };
        throw std::runtime_error( message );
    }

    m_binds[actionGroup].emplace( key, action );
}

auto core::KeyMapManager::onKeyPressed( const KeyPressedEvent& e ) -> void
{
}

auto core::KeyMapManager::setActiveGroup( const std::string& group ) -> void
{
    m_activeGroup = group;
}
