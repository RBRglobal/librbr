Introduction
============

API Concepts
------------

Object-Oriented Design
~~~~~~~~~~~~~~~~~~~~~~

The library adheres
to object-oriented design principles.
The core context object for instrument communications
is RBRGen3,
and RBRGen3Parser for dataset parsing.
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

RBRGen3 and RBRGen3Parser
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

Error Propagation
~~~~~~~~~~~~~~~~~

All but the most simple,
`functionally-pure <https://en.wikipedia.org/wiki/Purely_functional_programming>`__ functions
return an error indicator
of type :c:type:`RBRGen3Error`.
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
See the ``posixGen3/posix-stream.c`` example.

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
