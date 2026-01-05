add_library(kogayonon_physics
"include/physics/nvidia_physx.hpp"
"src/nvidia_physx.cpp"
)

target_include_directories(kogayonon_physics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_physics
PRIVATE spdlog::spdlog
PUBLIC unofficial::omniverse-physx-sdk::sdk
)
