#
# LIB_PREFIX: LWPRINTF
#
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWPRINTF_SYS_PORT: If defined, it will include port source file from the library.
# LWPRINTF_OPTS_FILE: If defined, it is the path to the user options file. If not defined, one will be generated for you automatically
# LWPRINTF_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWPRINTF_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

# Custom include directory
set(LWPRINTF_CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib_inc)

# Library core sources
set(lwprintf_core_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/lwprintf/lwprintf.c
)

# Add system port
if(DEFINED LWPRINTF_SYS_PORT)
    set(lwprintf_core_SRCS
        ${lwprintf_core_SRCS}
        ${CMAKE_CURRENT_LIST_DIR}/src/system/lwprintf_sys_${LWPRINTF_SYS_PORT}.c
    )
endif()

# Setup include directories
set(lwprintf_include_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    ${LWPRINTF_CUSTOM_INC_DIR}
)

# Register library to the system
add_library(lwprintf)
target_sources(lwprintf PRIVATE ${lwprintf_core_SRCS})
target_include_directories(lwprintf PUBLIC ${lwprintf_include_DIRS})
target_compile_options(lwprintf PRIVATE ${LWPRINTF_COMPILE_OPTIONS})
target_compile_definitions(lwprintf PRIVATE ${LWPRINTF_COMPILE_DEFINITIONS})

# Create config file if user didn't provide one info himself
if(NOT LWPRINTF_OPTS_FILE)
    message(STATUS "Using default lwprintf_opts.h file")
    set(LWPRINTF_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/src/include/lwprintf/lwprintf_opts_template.h)
else()
    message(STATUS "Using custom lwprintf_opts.h file from ${LWPRINTF_OPTS_FILE}")
endif()

configure_file(${LWPRINTF_OPTS_FILE} ${LWPRINTF_CUSTOM_INC_DIR}/lwprintf_opts.h COPYONLY)
