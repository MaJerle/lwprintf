.. _um_format_specifier:

Format specifier
================

Syntax
******

Full syntax for format specifier is
``%[parameter][flags][width][.precision][length]type``

Additional specifiers
*********************

LwPRINTF implementation supports some modifiers that are usually not available in standard implementation.
Those are more targeting embedded systems although they may be used in any general-purpose application

+-------------+--------------------------------------------------------------------------+
| Specifier   | Description                                                              |
+=============+==========================================================================+
| ``B`` ``b`` | Prints ``unsigned int`` data as binary representation.                   |
+-------------+--------------------------------------------------------------------------+
| ``K`` ``k`` | Prints ``unsigned char`` based data array as sequence of hex numbers.    |
|             | Use *width* field to specify length of input array.                      |
|             | Use ``K`` for uppercase hex letters, ``k`` for small.                    |
+-------------+--------------------------------------------------------------------------+

.. toctree::
    :maxdepth: 2

