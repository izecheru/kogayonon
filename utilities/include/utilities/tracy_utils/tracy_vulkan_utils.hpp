#pragma once

#ifndef TRACY_ENABLE
#define TracyVkZone( ctx, cmd, name )
#else
#include <tracy/TracyVulkan.hpp>
#endif