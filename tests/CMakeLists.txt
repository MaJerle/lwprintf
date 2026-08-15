cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

# Set default compile flags for GCC
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

enable_testing()
add_executable(${CMAKE_PROJECT_NAME})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test_main.c
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/
)

# Include file that can add more sources and prepare lib parameters
# May set LWPRINTF_TEST_ARCH to "32" or "64" if the test case has pointer-size
# assumptions baked into it; left undefined, the compiler's native width is
# used as-is.
include(${TEST_CMAKE_FILE_NAME})

# Force the pointer width the test case asked for (native GCC only -- the
# Win32-Debug/Win64-Debug mingw cross-compile presets already pick their
# width via a dedicated compiler binary and are untouched by this)
if(DEFINED LWPRINTF_TEST_ARCH AND CMAKE_C_COMPILER_ID STREQUAL "GNU" AND NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m${LWPRINTF_TEST_ARCH}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m${LWPRINTF_TEST_ARCH}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m${LWPRINTF_TEST_ARCH}")
endif()

# Select OS mutex port matching the host toolchain
if(WIN32)
    set(LWPRINTF_SYS_PORT win32)
else()
    set(LWPRINTF_SYS_PORT pthread)
endif()

# Add subdir with lwprintf and link to project
add_subdirectory("../lwprintf" lwprintf)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC lwprintf)
target_compile_definitions(lwprintf PUBLIC LWPRINTF_DEV)

if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(lwprintf PUBLIC Threads::Threads)
endif()

# Add test
add_test(NAME Test COMMAND $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)
