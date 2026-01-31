add_library(rmath STATIC
    src/rmath.cpp
)

if(NOT OpenCV_FOUND)
    find_package(OpenCV REQUIRED)
endif()

target_include_directories(rmath
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(rmath
    PUBLIC
        ${OpenCV_LIBS}
)
