cmake_minimum_required(VERSION 3.20)
project(Google_tests)
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Function to add test executables (Google Test + OpenCV)
function(add_test_executable test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE ${OpenCV_LIBS} gtest gtest_main)
endfunction()

add_test_executable(Google_test_run
        test_image_process.cpp
        ../src/image_process.cpp
)
