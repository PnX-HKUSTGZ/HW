# 指定CMake最低版本要求为3.16
cmake_minimum_required(VERSION 3.16)

# 设置工程名称为lightBarMatching01
project(lightBarMatching01)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)

# 查找OpenCV库
find_package(OpenCV REQUIRED)

# 查找yaml-cpp库
find_package(yaml-cpp REQUIRED)

# 查找Eigen3库版本为3.3，不使用模块方式
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# 生成可执行文件lightBarMatching01，链接detector.cpp和main.cpp文件
add_executable(lightBarMatching01 detector.cpp main.cpp)

# 将下面这行代码解注释，把上面的add_executable代码注释，再进行cmake 和 make之后得到的可执行文件的执行效果可以作为作业完成的指标（虽然这个版本参数还不是最好的）
# add_executable(lightBarMatching01 reference.cpp main.cpp)

# 链接所需的库 其中OpenCV_LIBS自动检测配置环境中的opencv文件路径， 
# CMAKE_CURRENT_SOURCE_DIR就是该文件所处目录路径，使用相对路径可以确保代码在不同设备迁移时仍能成功链接libMvCameraControl.so
target_link_libraries(
    lightBarMatching01 
    ${OpenCV_LIBS}
    yaml-cpp
    Eigen3::Eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/cameraOpen/64/libMvCameraControl.so
)