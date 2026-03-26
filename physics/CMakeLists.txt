add_library(physics
"include/physics/nvidia_physx.hpp"
"src/nvidia_physx.cpp"
)

target_include_directories(physics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(physics
PRIVATE spdlog::spdlog
PUBLIC unofficial::omniverse-physx-sdk::sdk
)
