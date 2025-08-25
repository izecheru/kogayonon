#pragma once
#include <any>
#include <map>
#include <memory>
#include <stdexcept>

#include "asset_manager/asset_manager.h"
#include "event/event_manager.h"
#include "klogger/klogger.h"
#include "renderer/camera.h"
#include "renderer/renderer.h"
#include "task/task_manager.h"

namespace kogayonon
{
enum class Context
{
  AssetManagerContext,
  TaskManagerContext,
  KLoggerContext,
  CameraContext, // this might get deleted from here since you might want multiple cameras
  RendererContext,
  EventManagerContext,
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
  case Context::EventManagerContext:
    return "Event";
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

  static std::shared_ptr<EventManager> event_manager()
  {
    return getFromContext<EventManager>(Context::EventManagerContext);
  }

private:
  inline static std::map<Context, std::any> context_map;
  inline static std::mutex m_context_map_mutex;
};
} // namespace kogayonon
