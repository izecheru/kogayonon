add_library(precompiled INTERFACE)

target_include_directories(precompiled INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_precompile_headers(precompiled INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/precompiled/pch.hpp"
)