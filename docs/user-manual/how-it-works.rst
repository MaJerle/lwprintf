.. _how_it_works:

How it works
============

LwPRINTF library supports ``2`` different formatting output types:

* Write formatted data to user input array
* Directly print formatted characters by calling ``output_function`` for every formatted character in the input string

Text formatting is based on input format string followed by the data parameters.
It is mostly used to prepare numeric data types to human readable format.

.. note::
    LwPRINTF is open-source implementation of regular *stdio.h* library in C language.
    It implements only output functions, excluding input scanning features

Formatting functions take input *format string* followed by (optional) different data types.
Internal algorithm scans character by character to understand type of expected data user would like to have printed.

Every format specifier starts with letter ``%``, followed by optional set of flags, widths and other sets of characters.
Last part of every specifier is its type, that being type of format and data to display.

.. tip::
    To print number ``1234`` in human readable format, use specifier ``%d``.
    With default configuration, call ``lwprintf_printf("%d", 1234);`` and it will print ``"1234"``.

Check section :ref:`um_format_specifier` for list of all formats and data types

.. toctree::
    :maxdepth: 2