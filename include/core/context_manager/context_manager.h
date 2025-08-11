#pragma once
#include <any>
#include <map>
#include <memory>
#include <stdexcept>

#include "core/asset_manager/asset_manager.h"
#include "core/klogger/klogger.h"
#include "core/task/task_manager.h"

namespace kogayonon
{
  enum class Context
  {
    AssetManagerContext,
    TaskManagerContext,
  };

  static std::string contextToString(Context id)
  {
    switch (id)
    {
    case Context::AssetManagerContext:
      return "AssetManager";
    case Context::TaskManagerContext:
      return "TaskManager";
    }
  }

  class ContextManager
  {
  public:
    ContextManager()                                 = delete;
    ContextManager(const ContextManager&)            = delete;
    ContextManager& operator=(const ContextManager&) = delete;

    template <typename T>
    static void addToContext(Context id, std::shared_ptr<T> context)
    {
      std::lock_guard lock(m_context_map_mutex);

      if (context_map.find(id) == context_map.end())
      {
        context_map.emplace(id, context);
        KLogger::log(LogType::CRITICAL, "Added to context - ", contextToString(id));
      }
      else
      {
        KLogger::log(LogType::CRITICAL, "Could not add to context - ", contextToString(id));
      }
    }

    template <typename T>
    static void removeFromContext(Context id)
    {
      std::lock_guard lock(m_context_map_mutex);
      if (context_map.find(id) != context_map.end())
      {
        context_map.erase(id);
        KLogger::log(LogType::CRITICAL, "Removed from context - ", contextToString(id));
      }
      else
      {
        KLogger::log(LogType::CRITICAL, "Could not find context for removal - ", contextToString(id));
      }
    }

    template <typename T>
    static std::shared_ptr<T> getFromContext(Context id)
    {
      auto it = context_map.find(id);
      if (it == context_map.end())
        throw std::runtime_error("Context not found");

      return std::any_cast<std::shared_ptr<T>>(it->second);
    }

    static void clear()
    {
      std::lock_guard lock(m_context_map_mutex);
      context_map.clear();
      KLogger::log(LogType::INFO, "ContextManager cleared all contexts.");
    }

    static std::shared_ptr<AssetManager> asset_manager()
    {
      return getFromContext<AssetManager>(Context::AssetManagerContext);
    }

    static std::shared_ptr<TaskManager> task_manager()
    {
      return getFromContext<TaskManager>(Context::TaskManagerContext);
    }

  private:
    inline static std::map<Context, std::any> context_map;
    inline static std::mutex m_context_map_mutex;
  };
} // namespace kogayonon
