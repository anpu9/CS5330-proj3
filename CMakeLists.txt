cmake_minimum_required(VERSION 3.20)
project(Proj3)

set(CMAKE_CXX_STANDARD 17)

# Base packages
find_package(OpenCV REQUIRED)

# Function for OpenCV-only executables
function(add_opencv_executable exe_name)
    add_executable(${exe_name} ${ARGN})
    target_link_libraries(${exe_name} PRIVATE ${OpenCV_LIBS})
    target_include_directories(${exe_name} PRIVATE ${OpenCV_INCLUDE_DIRS})
endfunction()



# Add OpenCV-only executables
add_opencv_executable(VidDisplay
        src/vidDisplay.cpp
        src/image_process.cpp
)
