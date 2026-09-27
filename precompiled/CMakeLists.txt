add_library(precompiled STATIC
    "src/pch.cpp"
    "include/precompiled/pch.hpp"
)

target_include_directories(precompiled PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_precompile_headers(precompiled PUBLIC
    "include/precompiled/pch.hpp"
)


