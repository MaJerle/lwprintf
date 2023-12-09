# 
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWPRINTF_SYS_PORT: If defined, it will include port source file from the library.
# LWPRINTF_OPTS_DIR: If defined, it should set the folder path where options file shall be generated.
# LWPRINTF_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWPRINTF_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

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
)

# Register library to the system
add_library(lwprintf INTERFACE)
target_sources(lwprintf INTERFACE ${lwprintf_core_SRCS})
target_include_directories(lwprintf INTERFACE ${lwprintf_include_DIRS})
target_compile_options(lwprintf PRIVATE ${LWPRINTF_COMPILE_OPTIONS})
target_compile_definitions(lwprintf PRIVATE ${LWPRINTF_COMPILE_DEFINITIONS})

# Create config file
if(DEFINED LWPRINTF_OPTS_DIR AND NOT EXISTS ${LWPRINTF_OPTS_DIR}/lwprintf_opts.h)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/src/include/lwprintf/lwprintf_opts_template.h ${LWPRINTF_OPTS_DIR}/lwprintf_opts.h COPYONLY)
endif()