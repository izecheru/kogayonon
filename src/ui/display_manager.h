#pragma once
#include <assert.h>

#include <memory>

#include "display.h"

namespace kogayonon
{
/**
 * @brief Utility class to add and remove displays from specific containers
 */
class DisplayManager
{
public:
  /**
   * @brief Adds a display to the specified container
   * @tparam T The type of Display
   * @tparam TContainer The type of container we pass (unordered_map and vector supported)
   * @param container unordered_map and vector supported (map for unique displays and vector for duplicates if needed)
   * @param d The display passed
   */
  template <typename TContainer>
  static void addDisplay(TContainer& container, std::unique_ptr<Display> d)
  {
    if constexpr (std::is_same_v<TContainer, std::vector<std::unique_ptr<Display>>>)
    {
      container.push_back(d);
    }
    else if constexpr (std::is_same_v<TContainer, std::unordered_map<std::string, std::unique_ptr<Display>>>)
    {
      container.emplace(d->getName(), std::move(d));
    }
    else
    {
      assert(sizeof(TContainer) == 0 && "unsuported container type, only unordered_map<string, unique_ptr<Display>");
    }
  }

  /**
   * @brief Removes a display from the specified container
   * @tparam T The type of Display
   * @tparam TContainer The type of container we pass (unordered_map and vector supported)
   * @param container unordered_map and vector supported (map for unique displays and vector for duplicates if needed)
   * @param d The display passed
   */
  template <typename TContainer>
  static void removeDisplay(TContainer& container, std::unique_ptr<Display> d)
  {
    if constexpr (std::is_same_v<TContainer, std::vector<std::unique_ptr<Display>>>)
    {
      container.erase(container.begin(), container.end(), d->getName());
    }
    else if constexpr (std::is_same_v<TContainer, std::unordered_map<std::string, std::unique_ptr<Display>>>)
    {
      if (auto& it = container.find(d->getName()); it != container.end())
      {
        container.erase(it);
      }
    }
    else
    {
      assert(sizeof(TContainer) == 0 && "unsuported container type, only unordered_map<string, unique_ptr<Display>");
    }
  }

private:
  DisplayManager() = delete;
  DisplayManager(const DisplayManager& other) = delete;
  DisplayManager operator=(const DisplayManager& other) = delete;
};
} // namespace kogayonon