cmake_minimum_required(VERSION 3.16)
project(opencv_test)

set(CMAKE_CXX_STANDARD 11)
find_package(OpenCV REQUIRED)


add_executable(opencv_test detector.cpp main.cpp)
target_link_libraries(opencv_test ${OpenCV_LIBS} )