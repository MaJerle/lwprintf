cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

if(NOT PROJECT_IS_TOP_LEVEL)
    add_subdirectory(lwprintf)
else()
    add_executable(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/dev/main.c
        ${CMAKE_CURRENT_LIST_DIR}/lwprintf/src/system/lwprintf_sys_win32.c
    )
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dev
    )

    # Add subdir with lwprintf and link to the project
    set(LWPRINTF_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/dev/lwprintf_opts.h)
    add_subdirectory(lwprintf)
    target_link_libraries(${PROJECT_NAME} lwprintf)

    # Add compile options to the library, which will propagate options to executable through public link
    target_compile_definitions(lwprintf PUBLIC WIN32 _DEBUG CONSOLE LWPRINTF_DEV)
    target_compile_options(lwprintf PUBLIC -Wall -Wextra -Wpedantic)
endif()
