cmake_minimum_required(VERSION 3.10)
project(Robot_Vison_Open_Vino)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
# Use modern OpenVINO package (matches <openvino/openvino.hpp> includes).
# If OpenVINO_DIR is missing or invalid, try common /opt/intel locations.
set(_openvino_candidate "")
if(OpenVINO_DIR AND EXISTS "${OpenVINO_DIR}/OpenVINOConfig.cmake")
  set(_openvino_candidate "${OpenVINO_DIR}")
elseif(EXISTS "/opt/intel/openvino_2025.4.0/runtime/cmake/OpenVINOConfig.cmake")
  set(_openvino_candidate "/opt/intel/openvino_2025.4.0/runtime/cmake")
elseif(EXISTS "/opt/intel/openvino_2025/runtime/cmake/OpenVINOConfig.cmake")
  set(_openvino_candidate "/opt/intel/openvino_2025/runtime/cmake")
endif()
if(_openvino_candidate)
  set(OpenVINO_DIR "${_openvino_candidate}" CACHE PATH "OpenVINO config dir")
endif()
unset(_openvino_candidate)
find_package(OpenVINO REQUIRED COMPONENTS Runtime)

# Build as a library (no main() in this repo).
add_library(Robot_Vison_Open_Vino STATIC OpenvinoInfer.cpp)
target_include_directories(Robot_Vison_Open_Vino
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${OpenCV_INCLUDE_DIRS}
)

if(TARGET openvino::runtime)
  target_link_libraries(Robot_Vison_Open_Vino PUBLIC ${OpenCV_LIBS} openvino::runtime)
elseif(TARGET OpenVINO::runtime)
  target_link_libraries(Robot_Vison_Open_Vino PUBLIC ${OpenCV_LIBS} OpenVINO::runtime)
else()
  message(FATAL_ERROR "OpenVINO runtime target not found. Check OpenVINO_DIR or installation.")
endif()

# Handle TBB dependency (OpenVINO runtime links to TBB).
set(_ov_root "")
if(OpenVINO_DIR)
  get_filename_component(_ov_runtime_dir "${OpenVINO_DIR}" DIRECTORY)
  get_filename_component(_ov_root "${_ov_runtime_dir}" DIRECTORY)
endif()

find_package(TBB QUIET)
if(TBB_FOUND)
  set(_ov_tbb_target TBB::tbb)
  target_link_libraries(Robot_Vison_Open_Vino PUBLIC ${_ov_tbb_target})
else()
  if(EXISTS "${_ov_root}/runtime/3rdparty/tbb/lib/libtbb.so")
    add_library(openvino_tbb SHARED IMPORTED)
    set_target_properties(openvino_tbb PROPERTIES
      IMPORTED_LOCATION "${_ov_root}/runtime/3rdparty/tbb/lib/libtbb.so"
    )
    set(_ov_tbb_target openvino_tbb)
    target_link_libraries(Robot_Vison_Open_Vino PUBLIC ${_ov_tbb_target})
  else()
    message(FATAL_ERROR "TBB not found. Install system TBB or set OpenVINO_DIR to a runtime that bundles TBB.")
  endif()
endif()

# Demo executable.
add_executable(Robot_Vison_Open_Vino_demo main.cpp)
target_link_libraries(Robot_Vison_Open_Vino_demo PRIVATE Robot_Vison_Open_Vino)
if(_ov_tbb_target)
  target_link_libraries(Robot_Vison_Open_Vino_demo PRIVATE ${_ov_tbb_target})
endif()

# Ensure runtime loader can find OpenVINO/TBB shared libs.
if(UNIX AND _ov_root)
  set_target_properties(Robot_Vison_Open_Vino_demo PROPERTIES
    BUILD_RPATH "${_ov_root}/runtime/lib/intel64;${_ov_root}/runtime/3rdparty/tbb/lib"
    INSTALL_RPATH "${_ov_root}/runtime/lib/intel64;${_ov_root}/runtime/3rdparty/tbb/lib"
  )
endif()
unset(_ov_root)
