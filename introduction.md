# Introduction

## API Concepts

### Object-Oriented Design

The library adheres
to object-oriented design principles.
The core context object for instrument communications
is RBRInstrument,
and RBRParser for dataset parsing.
The names of method functions
are prefixed with the name of the type
to which they apply,
and generally take an instance of that type
as their first argument.
This extends to many types
beyond just the core context objects;
for example,
all enum types have a corresponding “name” method
(e.g., #RBRInstrumentError and RBRInstrumentError_name()).

RBRInstrument and RBRParser
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

### Error Propagation

All but the most simple,
[functionally-pure][pure] functions
return an error indicator
of type #RBRInstrumentError.
Data is returned to the caller via out pointers.
This means that a common pattern
can be used for calling library functions
and either handling any error
or passing it further up the call stack:

~~~{.c}
RBRInstrumentError err;
RBRInstrumentFoo foo;
if ((err = RBRInstrument_foo(instrument, &foo)) != RBRINSTRUMENT_SUCCESS)
{
    return err;
}
/* Operate on foo. */
~~~

[pure]: https://en.wikipedia.org/wiki/Purely_functional_programming

### Memory Ownership

The only dynamic memory allocations
performed by the library
occur within RBRInstrument_open()
and RBRParser_init(),
as described in the documentation
for those functions.
In all other cases,
the buffers into which data
is to be returned
must be allocated by the caller.
This can be static, stack,
or heap allocation
at the caller's preference:
the important part is that
it is managed by the caller,
never by the library.

### Callbacks

The library isolates itself
from platform-specific tasks
(e.g., input/output)
by delegating these tasks
to callback functions
implemented by the library user.

### Streaming

Similarly, streaming data received from the instrument
while parsing other command responses
is forwarded to the user via a callback.
This lets the user receive streaming samples
without interrupting other instrument communication.
See the `posixGen3/posix-stream.c` example.

### Parsing

For consistency with
the streaming data model,
the parser also returns data via callbacks.
This enables convenient interleaving
of downloading and parsing,
and similar implementation of handling
for streamed and downloaded data.
See the `posixGen3/posix-parse-download.c` example.
