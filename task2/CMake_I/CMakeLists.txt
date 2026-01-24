# 在此处进行 CMake 设置（最小版本号，项目名）
cmake_minimum_required(VERSION 3.10)
project(Test)

find_package(OpenCV REQUIRED)

####### 以下禁止修改 #######
add_subdirectory(common)   #
add_subdirectory(modules)  #
####### 以上禁止修改 #######

# 在此处生成可执行程序
add_executable(test main.cpp)
# 在此处链接库
target_link_libraries(test PRIVATE common modules ${OpenCV_LIBS})
