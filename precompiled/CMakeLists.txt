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

#find_package(EASTL CONFIG REQUIRED)
#target_link_libraries(precompiled PRIVATE EASTL)


target_compile_options(precompiled PUBLIC
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>"
)


