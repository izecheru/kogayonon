#pragma once

#ifndef TRACY_ENABLE
#define ZoneScoped
#define ZoneScopedN( name )
#define ZoneScopedC( color )
#define ZoneScopedNC( name, color )

#define TracyVkZone( c, x, y )
#define TracyVkZoneC( c, x, y, z )
#define TracyVkZoneTransient( c, x, y, z, w )
#define TracyVkZoneS( c, x, y, z )
#define TracyVkZoneCS( c, x, y, z, w )
#define TracyVkZoneTransientS( c, x, y, z, w, a )
#else
#include "tracy/TracyVulkan.hpp"
#include <tracy/Tracy.hpp>
#endif