#pragma once
#include "utilities/task_manager/task.hpp"

namespace utilities
{
struct TaskRegistry
{
    std::vector<std::unique_ptr<enki::ITaskSet>> tasks;
    std::vector<std::unique_ptr<enki::ITaskSet>> dataTasks;
    std::vector<std::unique_ptr<enki::IPinnedTask>> pinnedTasks;
    std::vector<std::unique_ptr<enki::IPinnedTask>> pinnedDataTasks;
};
} // namespace utilities