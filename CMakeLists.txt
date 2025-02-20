
 # Authors: Yuyang Tian and Arun Mekkad
 # Date: 2025/2/14
 # Purpose: Build executable for Proj3 using cmake
 

cmake_minimum_required(VERSION 3.20)
project(Proj3)
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

set(CMAKE_CXX_STANDARD 17)
#  Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

 #  Add Google Test (Using the Submodule)
 # Ensure submodule is properly added and initialized
 set(GTEST_DIR ${CMAKE_SOURCE_DIR}/Google_tests/lib)
 add_subdirectory(${GTEST_DIR})  # Add the submodule
 include_directories(${GTEST_DIR}/googletest/include ${GTEST_DIR}/googlemock/include)

# Add MongoDB
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)
pkg_check_modules(BSONCXX REQUIRED libbsoncxx)

include_directories(${MONGOCXX_INCLUDE_DIRS} ${BSONCXX_INCLUDE_DIRS})
link_directories(${MONGOCXX_LIBRARY_DIRS} ${BSONCXX_LIBRARY_DIRS})


#  Function to add OpenCV-only executables
function(add_opencv_executable exe_name)
    add_executable(${exe_name} ${ARGN})
    target_link_libraries(${exe_name} PRIVATE ${OpenCV_LIBS})
endfunction()

# #  Function to add Test Executables (Google Test + OpenCV)
# function(add_test_executable test_name)
#     add_executable(${test_name} ${ARGN})
#     target_link_libraries(${test_name} PRIVATE ${OpenCV_LIBS} gtest gtest_main)
# endfunction()

# Function to add Executables (MongoDB + OpenCV + GoogleTest)
function(add_db_test_executable test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE
            ${OpenCV_LIBS}      # OpenCV support
            mongocxx bsoncxx    # MongoDB C++ driver
            gtest gtest_main    # GoogleTest
    )
    target_include_directories(${test_name} PRIVATE
            ${OpenCV_INCLUDE_DIRS}
            ${MONGOCXX_INCLUDE_DIRS}
            ${BSONCXX_INCLUDE_DIRS}
    )
endfunction()

function(add_db_executable test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE
            ${OpenCV_LIBS}      # OpenCV support
            mongocxx bsoncxx    # MongoDB C++ driver
    )
    target_include_directories(${test_name} PRIVATE
            ${OpenCV_INCLUDE_DIRS}
            ${MONGOCXX_INCLUDE_DIRS}
            ${BSONCXX_INCLUDE_DIRS}
    )
endfunction()

# 🔹 Add OpenCV executable
add_db_executable(VidDisplay
        src/vidDisplay.cpp
        src/image_process.cpp
        src/obb_feature_extraction.cpp
        db/db_manager.cpp
        db/db_config.cpp
        src/classifier.cpp
)

# 🔹 Add test executable
add_db_test_executable(Google_test_run
        tests/test_image_process.cpp  # Adjust the path to your test file
        src/image_process.cpp
        src/obb_feature_extraction.cpp
        db/db_manager.cpp
        db/db_config.cpp
)
