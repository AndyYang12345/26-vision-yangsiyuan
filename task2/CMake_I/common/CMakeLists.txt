add_library(common INTERFACE)

file(GLOB COMMON_CHILDREN RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(child ${COMMON_CHILDREN})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${child}" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt")
        add_subdirectory("${child}")
    endif()
endforeach()

if(TARGET math)
    target_link_libraries(common INTERFACE math)
endif()
if(TARGET kalman)
    target_link_libraries(common INTERFACE kalman)
endif()

set(Common_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/math/include;${CMAKE_CURRENT_LIST_DIR}/kalman/include"
    CACHE STRING "Common include directories")
