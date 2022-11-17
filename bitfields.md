# Working with Bit Fields

There are a few settings
where the instrument will self-report
which options are available
depending on the factory configuration
(for example, available serial baud rates,
or data memory storage formats).
When we anticipate that the number of possible options
will remain quite small,
we represent them
as an enumeration of bit flags.

The functions which report
the lists of options
all return a bit field
with the flags of the supported options set.
The availability of a specific option
can be confirmed using bitwise operators.

For example,
if all five streaming output formats
are available:

~~~{.c}
RBRInstrumentOutputFormat outputFormats;
RBRInstrument_getAvailableOutputFormats(instrument, &outputFormats);
assert(outputFormats == RBRINSTRUMENT_CALTEXT01
                      | RBRINSTRUMENT_CALTEXT02
                      | RBRINSTRUMENT_CALTEXT03
                      | RBRINSTRUMENT_CALTEXT04
                      | RBRINSTRUMENT_CALTEXT07);
~~~

And to check if a specific format is supported, e.g., `caltext03`:

~~~{.c}
RBRInstrumentOutputFormat outputFormats;
RBRInstrument_getAvailableOutputFormats(instrument, &outputFormats);
if (outputFormats | RBRINSTRUMENT_CALTEXT03)
{
    ...
}
~~~
