# Writing Tests

Unit tests are found
in the `testsGen3/` subdirectory.

## Adding an Instrument Test

You can use the `TEST_LOGGER2` and `TEST_LOGGER3` macros
to declare instrument test functions
within any of the test modules
found within `testsGen3/`.
The macros take a single argument:
the name of the test.

Tests are usually named after (or are prefixed with)
the instrument command being tested.
Test names collide only within a generation;
i.e., you may have both `TEST_LOGGER2(foo)`
and `TEST_LOGGER3(foo)`,
but you may not have two instances
of `TEST_LOGGER3(bar)`.
You do not need to declare test functions
in a header;
test functions defined
using these macros
will be automatically detected
at build time.

Functions declared with these macros
receive two arguments:
`instrument`, the test instrument connection;
and `buffers`, the I/O buffers for the instrument.
Test functions return a boolean indicating pass/fail.

For an example of a straightforward test,
let's consider the test for the `id` command:

~~~{.c}
TEST_LOGGER3(id)
{
    /* This is the result we expect from command parsing. */
    RBRGen3Id expected = {
        .model = "RBRduo3",
        .version = "1.092",
        .serial = 923456,
        .fwtype = 104
    };
    RBRGen3Id actual;

    /* Populate the read buffer with the command response. */
    TestIOBuffers_init(buffers,
                       "id model = RBRduo3, version = 1.092, "
                       "serial = 923456, fwtype = 104" COMMAND_TERMINATOR,
                       0);
    /* Get the test instrument connection to send/parse the command. */
    RBRGen3Error err = RBRGen3_getId(instrument, &actual);
    /* Check that the command sent matches our expectation. */
    TEST_ASSERT_STR_EQ("id" COMMAND_TERMINATOR, buffers->writeBuffer);
    /* Check the return value. */
    TEST_ASSERT_ENUM_EQ(RBRGEN3_SUCCESS, err, RBRGen3Error);
    /* Check the struct members. */
    TEST_ASSERT_STR_EQ(expected.model, actual.model);
    TEST_ASSERT_STR_EQ(expected.version, actual.version);
    TEST_ASSERT_EQ(expected.serial, actual.serial, "%" PRIi32);
    TEST_ASSERT_EQ(expected.fwtype, actual.fwtype, "%" PRIi32);

    /* If none of the previous tests failed, the test passes. */
    return true;
}
~~~

## Adding a Parser Test

Parser tests are similar to instrument tests:
use the `TEST_PARSER_CONFIG` macro
to declare a parser configuration,
then use the `TEST_PARSER` macro
to declare the test function.

The `TEST_PARSER_CONFIG` macro takes one argument,
the name of the config,
and takes the place of type declaration.
The same config can be reused
for multiple tests.

The `TEST_PARSER` macro takes two arguments:
the name of the test, and the name of the config.
As with instrument tests,
functions declared with these macros
receive two arguments:
`parser`, the test parser instance;
and `buffers`, the output buffers for the parser.
Test functions return a boolean indicating pass/fail.

For example,

~~~{.c}
TEST_PARSER_CONFIG(two_channels) = {
    .format = RBRGEN3_MEMFORMAT_CALBIN00,
    .formatConfig = {
        .easyParse = {
            .channels = 2
        }
    }
};

TEST_PARSER(test_a, two_channels)
{
    /* ... */
    return true;
}

TEST_PARSER(test_b, two_channels)
{
    /* ... */
    return true;
}
~~~


## Adding a Test Module

Test modules currently correspond exactly
to documentation sections of instrument functionality.
If you need to add a new test,
it likely fits within one of the existing modules.
However, if you're sure you do need a new module,
then you can add one
by creating a `.c` file in the `testsGen3/` subdirectory,
then adding its name (without extension)
to the `TEST_MODULES` variable in the `Makefile`.
Unless you have good reason not to,
try to retain alphabetical ordering
of the module names in the declaration.

For example,
to add a new module
for tests having to do with “frobbing”,
create the file `testsGen3/frobbing.c`,
and add it to the Makefile:

~~~
TEST_MODULES := communication \
                ...
                fetching \
                frobbing \
                gating \
                ...
~~~
