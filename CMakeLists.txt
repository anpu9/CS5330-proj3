cmake_minimum_required(VERSION 3.20)
project(Proj3)

set(CMAKE_CXX_STANDARD 17)

#  Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#  Add Google Test (Using the Submodule)
# Ensure submodule is properly added and initialized
set(GTEST_DIR ${CMAKE_SOURCE_DIR}/Google_tests/lib)
add_subdirectory(${GTEST_DIR})  # Add the submodule
include_directories(${GTEST_DIR}/googletest/include ${GTEST_DIR}/googlemock/include)

#  Function to add OpenCV-only executables
function(add_opencv_executable exe_name)
    add_executable(${exe_name} ${ARGN})
    target_link_libraries(${exe_name} PRIVATE ${OpenCV_LIBS})
endfunction()

#  Function to add Test Executables (Google Test + OpenCV)
function(add_test_executable test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE ${OpenCV_LIBS} gtest gtest_main)
endfunction()

# 🔹 Add OpenCV executable
add_opencv_executable(VidDisplay
        src/vidDisplay.cpp
        src/image_process.cpp
)

# 🔹 Add test executable
add_test_executable(Google_test_run
        tests/test_image_process.cpp  # Adjust the path to your test file
        src/image_process.cpp
        src/obb_feature_extraction.cpp
)