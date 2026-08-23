#pragma once
#include "precompiled/pch.hpp"

namespace core
{
// Where the action took place
enum class ActionLocation
{
  None,
};

// Action data
struct IAction
{
  void* data{ nullptr };
};

class UndoRedoSystem
{
public:

private:
  std::unordered_map<ActionLocation, std::queue<IAction>> m_actions;
};
} // namespace core
