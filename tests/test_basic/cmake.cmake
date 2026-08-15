# CMake include file

# Add more sources
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test_basic.c
)

# Options file
set(LWPRINTF_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/lwprintf_opts.h)

# The %p tests hardcode 32-bit pointer hex-width assertions
set(LWPRINTF_TEST_ARCH 32)
