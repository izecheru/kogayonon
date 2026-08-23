set(SOURCE_FILES
"src/entity_tests.cpp"
"src/font_loader_tests.cpp"
)
add_executable(test_runner 
${SOURCE_FILES}
)
#target_include_directories(test_runner PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_link_libraries(test_runner PRIVATE GTest::gtest GTest::gtest_main core resources utilities renderer)
include(GoogleTest)
gtest_discover_tests(test_runner)
