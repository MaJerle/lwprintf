cmake_minimum_required(VERSION 3.22)

# Register core library
add_library(lwprintf INTERFACE)
target_sources(lwprintf PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/lwprintf/lwprintf.c)
target_include_directories(lwprintf INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)

# Add system port
if(DEFINED LWPRINTF_SYS_PORT)
    target_sources(lwprintf PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/system/lwprintf_sys_${LWPRINTF_SYS_PORT}.c)
endif()

# Create config file
if(DEFINED LWPRINTF_OPTS_DIR AND NOT EXISTS ${LWPRINTF_OPTS_DIR}/lwprintf_opts.h)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/src/include/lwprintf/lwprintf_opts_template.h ${LWPRINTF_OPTS_DIR}/lwprintf_opts.h COPYONLY)
endif()