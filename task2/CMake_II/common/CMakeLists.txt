add_library(common INTERFACE)
file(GLOB COMMON_CHILDREN RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(child ${COMMON_CHILDREN})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${child}" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt")
        add_subdirectory("${child}")
    endif()
endforeach()

add_library(singleton INTERFACE)
target_include_directories(singleton INTERFACE "${CMAKE_CURRENT_LIST_DIR}/singleton/include")

if(TARGET rmath)
    target_link_libraries(common INTERFACE rmath)
endif()
target_link_libraries(common INTERFACE singleton)

set(Common_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/rmath/include;${CMAKE_CURRENT_LIST_DIR}/singleton/include"
    CACHE STRING "Common include directories")
