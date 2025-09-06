add_library(kogayonon_utilities
"include/utilities/task_manager/task_manager.h"
"src/task_manager.cpp"
)
target_include_directories(kogayonon_utilities PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_utilities PRIVATE kogayonon_logger)