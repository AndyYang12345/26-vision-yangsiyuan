add_library(kalman INTERFACE)

if(NOT OpenCV_FOUND)
    find_package(OpenCV REQUIRED)
endif()

target_include_directories(kalman
    INTERFACE
        "${CMAKE_CURRENT_LIST_DIR}/include"
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(kalman
    INTERFACE
        ${OpenCV_LIBS}
)

set(kalman_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/include"
    CACHE STRING "Kalman filter include directories")
