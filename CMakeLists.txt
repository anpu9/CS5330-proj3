cmake_minimum_required(VERSION 3.20)
project(Proj3)

set(CMAKE_CXX_STANDARD 17)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Add Google Test from local directory
add_subdirectory(Google_tests/lib)  # Points to your local GTest source
include_directories(Google_tests/lib/googletest/include)
include_directories(Google_tests/lib/googlemock/include)

# Function to add OpenCV-only executables
function(add_opencv_executable exe_name)
    add_executable(${exe_name} ${ARGN})
    target_link_libraries(${exe_name} PRIVATE ${OpenCV_LIBS})
endfunction()




# Add OpenCV executable
add_opencv_executable(VidDisplay
        src/vidDisplay.cpp
        src/image_process.cpp
)
