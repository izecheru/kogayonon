#pragma once

#ifdef _DEBUG
#define K_INFO( ... ) printf_s( "\033[32m[I]\033[m %s\n", std::format( __VA_ARGS__ ).c_str() )
#else
#define K_INFO( ... )
#endif

#ifdef _DEBUG
#define K_WARN( ... ) printf_s( "\033[33m[W]\033[m %s\n", std::format( __VA_ARGS__ ).c_str() )
#else
#define K_WARN( ... )
#endif

#ifdef _DEBUG
#define K_ERROR( ... )                                                                                                 \
  printf_s( "\033[31m[E]\033[m %s file: %s line:%d\n", std::format( __VA_ARGS__ ).c_str(), __FILE__, __LINE__ )
#else
#define K_ERROR( ... )
#endif

#ifdef _DEBUG
#define K_IF_ERROR( condition, message )                                                                               \
  if ( condition )                                                                                                     \
  {                                                                                                                    \
    K_ERROR( message );                                                                                                \
  }
#else
#define K_IF_ERROR( ... )
#endif

#ifdef _DEBUG
#define K_THROW( condition, ... )                                                                                      \
  if ( condition )                                                                                                     \
  {                                                                                                                    \
    std::throw runtime_error( __VA_ARGS__ );                                                                           \
  }
#else
#define K_THROW( condition, ... )
#endif

#ifdef _DEBUG
#define K_ASSERT( ... ) assert( __VA_ARGS__ );
#else
#define K_ASSERT( ... )
#endif