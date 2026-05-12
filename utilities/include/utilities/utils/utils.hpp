#pragma once

#ifndef NDEBUG
#define KOGAYONON_INFO( ... )                                                                                          \
  do                                                                                                                   \
  {                                                                                                                    \
    spdlog::info( __VA_ARGS__ );                                                                                       \
  } while ( 0 )
#else
#define KOGAYONON_INFO( x )                                                                                            \
  do                                                                                                                   \
  {                                                                                                                    \
  } while ( 0 )
#endif

#ifndef NDEBUG
#define KOGAYONON_WARN( ... )                                                                                          \
  do                                                                                                                   \
  {                                                                                                                    \
    spdlog::warn( __VA_ARGS__ );                                                                                       \
  } while ( 0 )
#else
#define KOGAYONON_WARN( x )                                                                                            \
  do                                                                                                                   \
  {                                                                                                                    \
  } while ( 0 )
#endif

#ifndef NDEBUG
#define KOGAYONON_ERR( ... )                                                                                           \
  do                                                                                                                   \
  {                                                                                                                    \
    spdlog::error( __VA_ARGS__ );                                                                                      \
  } while ( 0 )
#else
#define KOGAYONON_ERR( x )                                                                                             \
  do                                                                                                                   \
  {                                                                                                                    \
  } while ( 0 )
#endif