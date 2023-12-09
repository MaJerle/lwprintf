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