#include "renderer/blackboard.hpp"

template <typename T, typename... Args>
inline auto rendering::Blackboard::addToStorage( Args&&... args ) -> void
{
    constexpr entt::id_type hash = entt::type_hash<T>::value();
    if ( m_storage.contains( hash ) )
    {
        return;
    }

    m_storage[hash] = entt::meta_any( T( std::forward<Args>( args )... ) );
}

template <typename T>
inline T& rendering::Blackboard::get()
{
    return m_storage[entt::type_hash<T>::value()].cast<T&>();
}

template <typename T>
inline auto rendering::Blackboard::removeFromStorage() -> void
{
    m_storage.erase( entt::type_hash<T>::value() );
}
