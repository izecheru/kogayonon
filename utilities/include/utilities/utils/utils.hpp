#pragma once

#ifdef _DEBUG
#define KINFO( ... ) printf_s( "\033[32m[I]\033[m %s\n", std::format( __VA_ARGS__ ).c_str() )
#else
#define KINFO( ... )
#endif

#ifdef _DEBUG
#define KWARN( ... ) printf_s( "\033[33m[W]\033[m %s\n", std::format( __VA_ARGS__ ).c_str() )
#else
#define KWARN( ... )
#endif

#ifdef _DEBUG
#define KERROR( ... )                                                                                                  \
    printf_s( "\033[31m[E]\033[m %s file: %s line:%d\n", std::format( __VA_ARGS__ ).c_str(), __FILE__, __LINE__ )
#else
#define KERROR( ... )
#endif

#ifdef _DEBUG
#define K_IF_ERROR( condition, message )                                                                               \
    if ( condition )                                                                                                   \
    {                                                                                                                  \
        KERROR( message );                                                                                             \
    }
#else
#define K_IF_ERROR( ... )
#endif

#ifdef _DEBUG
#define KTHROW( condition, ... )                                                                                       \
    if ( condition )                                                                                                   \
    {                                                                                                                  \
        std::throw runtime_error( __VA_ARGS__ );                                                                       \
    }
#else
#define KTHROW( condition, ... )
#endif

#ifdef _DEBUG
#define KASSERT( ... ) assert( __VA_ARGS__ );
#else
#define KASSERT( ... )
#endif