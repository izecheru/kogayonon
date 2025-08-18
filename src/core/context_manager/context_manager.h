#pragma once
#include <any>
#include <map>
#include <memory>
#include <stdexcept>

#include "core/asset_manager/asset_manager.h"
#include "core/klogger/klogger.h"
#include "core/renderer/camera.h"
#include "core/renderer/renderer.h"
#include "core/task/task_manager.h"

namespace kogayonon
{
  enum class Context
  {
    AssetManagerContext,
    TaskManagerContext,
    KLoggerContext,
    CameraContext,
    RendererContext
  };

  static std::string contextToString(Context id)
  {
    switch (id)
    {
    case Context::AssetManagerContext:
      return "AssetManager";
    case Context::TaskManagerContext:
      return "TaskManager";
    case Context::KLoggerContext:
      return "KLogger";
    case Context::CameraContext:
      return "Camera";
    case Context::RendererContext:
      return "Renderer";
    }
    return "Not Found";
  }

  class ContextManager
  {
  public:
    ContextManager() = delete;
    ContextManager(const ContextManager&) = delete;
    ContextManager& operator=(const ContextManager&) = delete;

    template <typename T>
    static void addToContext(Context id, std::shared_ptr<T> context)
    {
      std::lock_guard lock(m_context_map_mutex);

      if (context_map.find(id) == context_map.end())
      {
        context_map.emplace(id, context);
        klogger()->log(LogType::CRITICAL, "Added to context - ", contextToString(id));
      }
      else
      {
        klogger()->log(LogType::CRITICAL, "Could not add to context - ", contextToString(id));
      }
    }

    template <typename T>
    static void removeFromContext(Context id)
    {
      std::lock_guard lock(m_context_map_mutex);
      if (context_map.find(id) != context_map.end())
      {
        context_map.erase(id);
        klogger()->log(LogType::CRITICAL, "Removed from context - ", contextToString(id));
      }
      else
      {
        klogger()->log(LogType::CRITICAL, "Could not find context for removal - ", contextToString(id));
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
    }

    static std::shared_ptr<Renderer> renderer()
    {
      return getFromContext<Renderer>(Context::RendererContext);
    }

    static std::shared_ptr<AssetManager> asset_manager()
    {
      return getFromContext<AssetManager>(Context::AssetManagerContext);
    }

    static std::shared_ptr<TaskManager> task_manager()
    {
      return getFromContext<TaskManager>(Context::TaskManagerContext);
    }

    static std::shared_ptr<KLogger> klogger()
    {
      return getFromContext<KLogger>(Context::KLoggerContext);
    }

    static std::shared_ptr<Camera> camera()
    {
      return getFromContext<Camera>(Context::CameraContext);
    }

  private:
    inline static std::map<Context, std::any> context_map;
    inline static std::mutex m_context_map_mutex;
  };
} // namespace kogayonon
