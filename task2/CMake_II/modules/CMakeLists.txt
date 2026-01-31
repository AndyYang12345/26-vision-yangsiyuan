add_library(assembly1 STATIC
    assembly1/src/assembly1.cpp
)
target_include_directories(assembly1 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/assembly1/include")

add_library(assembly2 STATIC
    assembly2/src/assembly2.cpp
)
target_include_directories(assembly2 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/assembly2/include" ${Common_INCLUDE_DIRS})

add_library(module1 STATIC
    module1/src/module1.cpp
)
if(BUILD_A)
    target_link_libraries(module1 PUBLIC assembly1)
elseif(BUILD_B)
    target_link_libraries(module1 PUBLIC assembly2)
endif()

add_library(module2 STATIC
    module2/src/module2.cpp
)
target_include_directories(module2 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/module2/include" ${Common_INCLUDE_DIRS})