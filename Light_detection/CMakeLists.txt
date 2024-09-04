cmake_minimum_required(VERSION 3.10)
project(Light_detection)

## Use C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## By adding -Wall and -Werror, the compiler does not ignore warnings anymore,
## enforcing cleaner code.
add_definitions(-Wall -Werror)

## Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#######################
## Find dependencies ##
#######################

find_package(OpenCV REQUIRED)

###########
## Build ##
###########
include_directories(${CMAKE_SOURCE_DIR}/includes)
add_executable(Light_detection src/Light_detection.cpp)
target_include_directories(Light_detection PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(Light_detection ${OpenCV_LIBS})

#############
## Testing ##
#############
