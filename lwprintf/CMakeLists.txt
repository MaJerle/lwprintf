cmake_minimum_required(VERSION 3.13)

# Debug message
message("LWPRINTF: Entering lib source CMakeLists.txt")

# Set library name
set(LWPRINTF_LIB_NAME "lwprintf")

# Check definitions of some key things
option(LWPRINTF_SYS_ADD_FILES OFF)
if (LWPRINTF_SYS_ADD_FILES)
    message("LWPRINTF: LWPRINTF_SYS_ADD_FILES is set to ON. Looking for LWPRINTF_SYS_ARCH definition")
    if(NOT DEFINED LWPRINTF_SYS_ARCH)
        message("LWPRINTF: LWPRINTF_SYS_ARCH not defined. Using default one: \"win32\"")
        set(LWPRINTF_SYS_ARCH "win32")
    endif()
    message("LWPRINTF: LWPRINTF_SYS_ARCH set to: " ${LWPRINTF_SYS_ARCH})
else()
    message("LWPRINTF: LWPRINTF_SYS_ADD_FILES is set to OFF. User must manually implement system functions in case of operating system support")
endif()

# Register library to the system
add_library(${LWPRINTF_LIB_NAME} INTERFACE)

# Setup generic source files
target_sources(${LWPRINTF_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/lwprintf/lwprintf.c
    )

# Add optional sources
if (LWPRINTF_SYS_ADD_FILES)
target_sources(${LWPRINTF_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/system/lwprintf_sys_${LWPRINTF_SYS_ARCH}.c
    )
endif()

# Setup include directories
target_include_directories(${LWPRINTF_LIB_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    )