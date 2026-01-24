add_library(math STATIC
    src/Math.cpp
)

if(NOT OpenCV_FOUND)
    find_package(OpenCV REQUIRED)
endif()

target_include_directories(math
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include"
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(math
    PUBLIC
        ${OpenCV_LIBS}
)
