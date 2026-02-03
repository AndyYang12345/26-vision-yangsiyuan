cmake_minimum_required(VERSION 3.10)
project(task1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(OpenCV REQUIRED)

add_library(task1_core
  src/detector/ArmorDetectionPipeline.cpp
  src/detector/ArmorClassifier.cpp
  src/detector/ArmorDetector.cpp
  src/detector/LightBarDetector.cpp
  src/solver/PnPSolver.cpp
  src/preprocessor/ColorPreprocessor.cpp
  src/preprocessor/ImagePreprocessor.cpp
  src/types/Armor.cpp
  src/types/LightBar.cpp
)

target_include_directories(task1_core
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(task1_core
  PUBLIC
    ${OpenCV_LIBS}
)

add_library(task1_tuner
  src/tuner/ColorProcessorTuner.cpp
)
target_link_libraries(task1_tuner PUBLIC task1_core)

option(TASK1_BUILD_APP "Build task1 executable" ON)
if(TASK1_BUILD_APP)
  add_executable(task1 app/main.cpp)
  target_link_libraries(task1 PRIVATE task1_core)
endif()

option(TASK1_BUILD_COLOR_TUNER "Build simple_color_tuner executable" ON)
if(TASK1_BUILD_COLOR_TUNER)
  add_executable(simple_color_tuner app/simple_color_tuner.cpp)
  target_link_libraries(simple_color_tuner PRIVATE task1_tuner)
endif()
