# Lightweight printf stdio manager

[Open documentation](https://docs.majerle.eu/projects/lwprintf/)

## Features

* Written in C (C11), compatible with ``size_t`` and ``uintmax_t`` types for some specifiers
* Implements output functions compatible with ``printf``, ``vprintf``, ``snprintf``, ``sprintf`` and ``vsnprintf``
* Low-memory footprint, suitable for embedded systems
* Reentrant access to all API functions
* Operating-system ready
* Requires single output function to be implemented by user for ``printf``-like API calls
* With optional functions for operating systems to protect multiple threads printing to the same output stream
* Allows multiple output stream functions (unlike standard ``printf`` which supports only one) to separate parts of application
* Added additional specifiers vs original features
* User friendly MIT license

## Contribute

Fresh contributions are always welcome. Simple instructions to proceed:

1. Fork Github repository
2. Follow [C style & coding rules](https://github.com/MaJerle/c-code-style) and use `clang-format` to format the code
3. Create a pull request to `develop` branch with new features or bug fixes

Alternatively you may:

1. Report a bug
2. Ask for a feature request
