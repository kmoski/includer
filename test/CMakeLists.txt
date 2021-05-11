find_package(GTest)
enable_testing()

set(TEST_SOURCE main.cpp settings_parser.cpp)

add_executable(${PROJECT_NAME}_test)
target_sources(${PROJECT_NAME}_test
    PRIVATE ${TEST_SOURCE}
            $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_SOURCES>)
target_include_directories(${PROJECT_NAME}_test
    PRIVATE ${GTEST_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}_test
    gtest
    gtest_main
    pthread)