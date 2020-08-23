.. _thread_safety:

Thread safety
=============

With default configuration, LwPRINTF library is *not* thread safe.
This means whenever it is used with operating system, user must resolve it with care.

Library has locking mechanism support for thread safety, which needs to be enabled manually.

.. tip::
    To enable thread-safety support, parameter ``LWPRINTF_CFG_OS`` must be set to ``1``.
    Please check :ref:`api_lwprintf_opt` for more information about other options.

After thread-safety features has been enabled, it is necessary to implement
``4`` low-level system functions.

.. tip::
    System function template example is available in ``lwprintf/src/system/`` folder.

Example code for ``CMSIS-OS V2``

.. note::
    Check :ref:`api_lwprintf_sys` section for function description

.. literalinclude:: ../../lwprintf/src/system/lwprintf_sys_cmsis_os.c
    :language: c
    :linenos:
    :caption: System function implementation for CMSIS-OS based operating systems

.. toctree::
    :maxdepth: 2