add_library(physics
	"include/physics/jolt_physics.hpp"
	"include/physics/rigidbody_type.hpp"

	"src/jolt_physics.cpp"
)

target_include_directories(physics PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(physics
	PUBLIC	
	glm::glm-header-only
	spdlog::spdlog_header_only
	Jolt::Jolt
)
