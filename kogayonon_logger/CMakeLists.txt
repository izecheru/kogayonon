add_library(kogayonon_logger "include/logger/logger.h" "src/logger.cpp")

target_include_directories(kogayonon_logger
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
