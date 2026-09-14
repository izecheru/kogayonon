cmake_minimum_required(VERSION 3.20)

add_library(tinygltf STATIC)

target_sources(tinygltf PRIVATE 
    "src/tiny_gltf_v3.c"
)

target_include_directories(tinygltf PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

target_compile_definitions(tinygltf PRIVATE
    TINYGLTF3_ENABLE_FS        
)

set_target_properties(tinygltf PROPERTIES
    C_STANDARD 11              
    C_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(tinygltf PRIVATE /Zc:preprocessor)
endif()
