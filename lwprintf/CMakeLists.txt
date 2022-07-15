cmake_minimum_required(VERSION 3.22)

# Debug message
message("Entering ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")

# Register core library
add_library(lwprintf INTERFACE)
target_sources(lwprintf PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/lwprintf/lwprintf.c)
target_include_directories(lwprintf INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)

if (DEFINED LWPRINTF_SYS_PORT)
target_sources(lwprintf PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/system/lwprintf_sys_${LWPRINTF_SYS_PORT}.c)
endif()

# Debug message
message("Exiting ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")
