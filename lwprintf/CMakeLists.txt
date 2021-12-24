cmake_minimum_required(VERSION 3.13)

# Debug message
message("LWPRINTF: Entering lib source CMakeLists.txt")

# Set library name
set(LWPRINTF_LIB_NAME "lwprintf")

# Check definitions of some key things
if(NOT DEFINED LWMEM_SYS_ARCH)
    message("LWPRINTF: LWPRINTF_SYS_ARCH not defined. Using default one: \"win32\"")
    set(LWMEM_SYS_ARCH "win32")
endif()

# Register library to the system
add_library(${LWPRINTF_LIB_NAME} INTERFACE)

# Setup generic source files
target_sources(${LWPRINTF_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/lwprintf/lwprintf.c
    ${CMAKE_CURRENT_LIST_DIR}/src/system/lwprintf_sys_${LWMEM_SYS_ARCH}.c
    )

# Setup include directories
target_include_directories(${LWPRINTF_LIB_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    )
