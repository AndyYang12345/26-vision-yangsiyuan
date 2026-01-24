add_library(A1 STATIC
    A1/src/A11.cpp
    A1/src/A12.cpp
    A1/src/A13.cpp
)
target_include_directories(A1 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/A1/include")

add_library(A2 STATIC
    A2/src/A2.cpp
)
target_include_directories(A2 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/A2/include")

add_library(M1 STATIC
    M1/src/M1.cpp
)
target_include_directories(M1 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/M1/include")
target_link_libraries(M1 PUBLIC A1)

add_library(M2 STATIC
    M2/src/M2.cpp
)
target_include_directories(M2 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/M2/include")
target_link_libraries(M2 PUBLIC A1 A2 kalman)

add_library(modules INTERFACE)
target_link_libraries(modules INTERFACE M1 M2)
