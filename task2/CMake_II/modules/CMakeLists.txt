if(NOT OpenCV_FOUND)
    find_package(OpenCV REQUIRED)
endif()

add_library(assembly1 STATIC
    assembly1/src/assembly1.cpp
)
target_include_directories(assembly1 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/assembly1/include")

add_library(assembly2 STATIC
    assembly2/src/assembly2.cpp
)
target_include_directories(assembly2 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/assembly2/include")
target_link_libraries(assembly2 PUBLIC ${OpenCV_LIBS})

if(BUILD_A AND BUILD_B)
    message(FATAL_ERROR "BUILD_A and BUILD_B cannot both be ON")
endif()
if(NOT BUILD_A AND NOT BUILD_B)
    message(FATAL_ERROR "Either BUILD_A or BUILD_B must be ON")
endif()

set(MODULE1_SOURCES "")
if(BUILD_A)
    list(APPEND MODULE1_SOURCES
        module1/src/module1a/m1.cpp
        module1/src/module1a/m2.cpp
        module1/src/module1a/m3.cpp
        module1/src/module1a/m4.cpp
    )
elseif(BUILD_B)
    list(APPEND MODULE1_SOURCES
        module1/src/module1b/module1b.cpp
    )
endif()

add_library(module1 STATIC ${MODULE1_SOURCES})
target_include_directories(module1 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/module1/include")
if(BUILD_A)
    target_compile_definitions(module1 PUBLIC WITH_A)
elseif(BUILD_B)
    target_compile_definitions(module1 PUBLIC WITH_B)
endif()
if(BUILD_B)
    target_link_libraries(module1 PUBLIC assembly2)
endif()

add_library(module2 STATIC
    module2/src/client.cpp
    module2/src/server.cpp
    module2/src/object.cpp
    module2/src/variable.cpp
    module2/src/argument.cpp
    module2/src/open62541.c
)
target_include_directories(module2 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/module2/include")
target_link_libraries(module2 PUBLIC pthread)

if(BUILD_TESTS)
    add_executable(assembly1_test assembly1/test/assembly1_test.cpp)
    target_link_libraries(assembly1_test PRIVATE assembly1)
    add_test(NAME assembly1_test COMMAND assembly1_test)
endif()
