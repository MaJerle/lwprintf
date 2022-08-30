# Changelog

## Develop

- da808ecf0122a6a11eaf896fd1025c1e4f1e0ce8 Fix calculation for NULL terminated string and precision with 0 as an input
- Split CMakeLists.txt files between library and executable
- Fix missing break in switch statement
- Add support for manual mutual-exclusion setup in OS mode
- Change license year to 2022
- Update code style with astyle
- Add `.clang-format` draft

## v1.0.3

- CMSIS-OS improvements for Kernel aware debuggers

## v1.0.2

- Fixed `float` output when engineering mode is disabled

## v1.0.1

- Fixed compiler error when engineering mode disabled but float enabled
- Properly handled `zero` float inputs

## v1.0.0

- First stable release
- Embedded systems optimized library
- Apply all modifiers except `%a`
- Extensive docs available
- Operating system ready with CMSIS-OS template
