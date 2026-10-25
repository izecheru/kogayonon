#pragma once
#include "utilities/input/key_codes.hpp"

namespace core
{
class KeyPressedEvent;
}

namespace core
{
class EventDispatcher;

enum EAction : uint32_t
{
    MoveForward,
    MoveBack,
    MoveLeft,
    MoveRight,
    Jump,
    Sprint,
    Crouch
};

using TActionMap = std::unordered_map<std::string, std::unordered_map<EAction, std::function<void()>>>;
using TActionBindMap = std::unordered_map<std::string, std::unordered_map<utilities::KeyScanCode, EAction>>;

class KeyMapManager
{
  public:
    explicit KeyMapManager( const std::filesystem::path& mapPath, EventDispatcher* dispatcher );
    explicit KeyMapManager( EventDispatcher* dispatcher );

    ~KeyMapManager() = default;

    /**
     * @brief Register action within a specific action group
     * @param actionGroup Group name
     * @param action
     * @param func
     * @return
     */
    auto registerAction( const std::string& actionGroup, EAction action, std::function<void()> func ) -> void;

    /**
     * @brief Bind a specific key to a specific action from within a specific group
     * @param actionGroup
     * @param key
     * @param action
     * @return
     */
    auto bindAction( const std::string& actionGroup, const utilities::KeyScanCode key, const EAction action ) -> void;

    /**
     * @brief Proceed to execute all the flagged callbacks within a specific action group
     * @param actionGroup
     * @return
     */
    auto updateGroup( const std::string& actionGroup ) -> void;

    auto setActiveGroup( const std::string& group ) -> void;

  private:
    auto onKeyPressed( const KeyPressedEvent& e ) -> void;

  private:
    TActionMap m_actions;
    TActionBindMap m_binds;
    std::filesystem::path m_mapPath;
    std::string m_activeGroup;
};
} // namespace core