.. Copyright (c) 2026 RBR Ltd.
.. SPDX-License-Identifier: Apache-2.0

Introduction
============

API Concepts
------------

Object-Oriented Design
~~~~~~~~~~~~~~~~~~~~~~

The library adheres
to object-oriented design principles.
Each generation has a core context object
for instrument communications:
:c:type:`RBRGen3` for Gen3
and :c:type:`RBRGen4` for Gen4.
Gen3 also has :c:type:`RBRGen3Parser` for dataset parsing;
Gen4's dataset parsing support
will be added in a future release of libRBR.
The names of method functions
are prefixed with the name of the type
to which they apply,
and generally take an instance of that type
as their first argument.
This extends to many types
beyond just the core context objects;
for example,
all enum types have a corresponding “name” method
(e.g., :c:type:`RBRGen3Error` and :c:func:`RBRGen3Error_name`).

:c:type:`RBRGen3`, :c:type:`RBRGen4` and :c:type:`RBRGen3Parser`
are the only struct types which leverage
the idea of getters and setters.
While these are not “opaque” types,
we strongly discourage direct modification
of their instance fields.
We've intentionally omitted setters
for some fields
because we want to retain the option
of making instances more stateful.
As the library matures
and we get a clearer picture
of how it's used
and how it needs to grow,
we're open to exposing more
of these internal structures
as the need presents.

Functions Map to Instrument Commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In general,
one function call
sends one command to the instrument
and expects one response from the instrument.
The underlying command is documented
in a ``\command`` tag in each function's
Doxygen comment.
There are, however,
several Gen3 exceptions to this rule
for large commands
which are better sent as multiple smaller chunks
(e.g. :c:func:`RBRGen3_setPostprocessing`)
or for convenience functions that perform multiple queries
(e.g. :c:func:`RBRGen3_getChannels`).

Read => Modify => Write
~~~~~~~~~~~~~~~~~~~~~~~

In general,
applications should call a command's getter function
to initialize the command's struct
before calling its setter function.
This is because setter functions
always send all *writable* parameters,
so leaving it initialized to a default value
may result in an error
(if the default value is invalid)
or an unexpected change to instrument state.

Even if all *writable* fields are assigned
before calling a setter,
it is still recommended to call the getter;
this future proofs the application against
additions to the API,
such as a new writable parameter for a command.

.. code-block:: c

    /* Read */
    RBRGen4Schedule schedule = {
        .label = "myschedule",
    };
    RBRGen4Error err = RBRGen4_getSchedule(&conn, &schedule, NULL);
    if (err) {
        goto cleanup;
    }
    /* Modify */
    schedule.mode = RBRGEN4_SCHEDULE_MODE_CONTINUOUS;
    schedule.parameters.continuous.period = 1000;
    const RBRGen4LabelList scheduleGroupList = {
        .size = 1,
        .len = 1,
        .labels = (RBRGen4Label[]){"mygroup"},
    };
    /* Write */
    err = RBRGen4_setSchedule(&conn, &schedule, &scheduleGroupList);
    if (err) {
        goto cleanup;
    }

Error Propagation
~~~~~~~~~~~~~~~~~

All but the most simple,
`functionally-pure <https://en.wikipedia.org/wiki/Purely_functional_programming>`__ functions
return an error indicator
of type :c:type:`RBRGen3Error` or :c:type:`RBRGen4Error`.
Data is returned to the caller via out pointers.
This means that a common pattern
can be used for calling library functions
and either handling any error
or passing it further up the call stack:

.. code-block:: c

   RBRGen3Error err;
   RBRGen3Foo foo;
   if ((err = RBRGen3_foo(conn, &foo)) != RBRGEN3_SUCCESS)
   {
       return err;
   }
   /* Operate on foo. */

Read-Only and Write-Only Members
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some members of parameter structs
are marked **Read-only** or **Write-only**.
A read-only member corresponds to a read-only instrument parameter:
it is populated
when reading parameters from the instrument,
and its value is ignored
when sending parameters to the instrument.
A write-only member is used only
when writing parameters to the instrument.

Memory Ownership
~~~~~~~~~~~~~~~~

The library never allocates memory.
Context objects
(:c:type:`RBRGen3`, :c:type:`RBRGen3Parser`,
:c:type:`RBRGen4`),
the command and response buffers
used for communication
(see :c:type:`RBRGen3Environment`, :c:type:`RBRGen4Environment`),
and the buffers into which data
is to be returned
must be allocated by the caller.
This can be static, stack,
or heap allocation
at the caller's preference:
the important part is that
it is managed by the caller,
never by the library.

Callbacks
~~~~~~~~~

The library isolates itself
from platform-specific tasks
(e.g., input/output)
by delegating these tasks
to callback functions
implemented by the library user.

Streaming
~~~~~~~~~

Similarly, streaming data received from the instrument
while parsing other command responses
is forwarded to the user via a callback.
This lets the user receive streaming samples
without interrupting other instrument communication.
See the ``posixGen3/posix-stream.c``
and ``posixGen4/posix-stream.c`` examples.

Parsing
~~~~~~~

For consistency with
the streaming data model,
the parser also returns data via callbacks.
This enables convenient interleaving
of downloading and parsing,
and similar implementation of handling
for streamed and downloaded data.
See the ``posixGen3/posix-parse-download.c`` example.

Generation Detection
~~~~~~~~~~~~~~~~~~~~

To identify an instrument's generation,
applications with the Gen3 API compiled
can call :c:func:`RBRGen3_open`;
it will return :c:enumerator:`RBRGEN3_UNSUPPORTED`
for instruments that are not compatible with Gen3,
at which point one can call :c:func:`RBRGen3_getGeneration`.
:c:func:`RBRGen4_open` will correctly refuse incompatible instruments,
but does not identify their generation.
See ``examples/posixMultiGen/posix-detect.c``.