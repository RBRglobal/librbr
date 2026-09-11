/**
 * \file tests.h
 *
 * \brief Common testing definitions, structures, and functions.
 *
 * End users shouldn't need to consume anything from this file, but library
 * developers will want to consult it for insight on test authoring.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_TESTS_H
#define LIBRBR_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Required for printf. */
#include <stdio.h>

#include <stdlib.h>
/* Required for strcmp, strlen. */
#include <string.h>

#include "RBRInstrumentGen4.h"
#include "RBRDynamicCorrectionGen4.h"
#include "RBRParserGen4.h"

/**
 * \brief Assert that a condition is true.
 *
 * If the assertion fails, an error message will be printed containing the file
 * name and line number on which the macro invocation occurs, and the
 * surrounding function will `return false;`.
 *
 * \param [in] _condition the condition to test
 */
#define TEST_ASSERT(_condition) do { \
        if (!(_condition)) \
        { \
            printf(" assertion failure at %s:%d", \
                   __FILE__, \
                   __LINE__); \
            return false; \
        } \
} while (0)

/**
 * \brief Assert that two variables are equal.
 *
 * If the assertion fails, an error message will be printed containing the file
 * name and line number on which the macro invocation occurs and the expected
 * and actual values, and the surrounding function will `return false;`.
 *
 * Because \a _expected, \a _actual, and \a _type will be evaluated multiple
 * times by the macro, do not pass expressions having side effects.
 *
 * \param [in] _expected the expected value
 * \param [in] _actual the actual value
 * \param [in] _type a printf format string suitable for the values
 */
#define TEST_ASSERT_EQ(_expected, _actual, _type) do { \
        if ((_expected) != (_actual)) \
        { \
            printf(" assertion failure at %s:%d:" \
                   " expected " _type "; actual " _type, \
                   __FILE__, \
                   __LINE__, \
                   _expected, \
                   _actual); \
            return false; \
        } \
} while (0)

/**
 * \brief Assert that two float variables are equal.
 *
 * If the assertion fails, an error message will be printed containing the file
 * name and line number on which the macro invocation occurs and the expected
 * and actual values, and the surrounding function will `return false;`.
 *
 * Because \a _expected, \a _actual, and \a _type will be evaluated multiple
 * times by the macro, do not pass expressions having side effects.
 *
 * \param [in] _expected the expected value
 * \param [in] _actual the actual value
 * \param [in] _eps the precision range for comparison
 */
#define TEST_ASSERT_FLOAT_EQ(_expected, _actual, _eps) do { \
        if (((_expected) < ((_actual) - (_eps))) \
            || ((_expected) > ((_actual) + (_eps)))) \
        { \
            printf(" assertion failure at %s:%d:" \
                   " expected %f ; actual %f", \
                   __FILE__, \
                   __LINE__, \
                   (double) (_expected), \
                   (double) (_actual)); \
            return false; \
        } \
} while (0)



/**
 * \brief Assert that two enum members are equal.
 *
 * If the assertion fails, an error message will be printed containing the file
 * name and line number on which the macro invocation occurs and the expected
 * and actual members, and the surrounding function will `return false;`.
 *
 * Because \a _expected, \a _actual, and \a _type will be evaluated multiple
 * times by the macro, do not pass expressions having side effects.
 *
 * \param [in] _expected the expected enum member
 * \param [in] _actual the actual enum member
 * \param [in] _enum the type name of the enum of which the values are members
 */
#define TEST_ASSERT_ENUM_EQ(_expected, _actual, _enum) do { \
        if ((_expected) != (_actual)) \
        { \
            printf(" assertion failure at %s:%d:" \
                   " expected %s; actual %s", \
                   __FILE__, \
                   __LINE__, \
                   _enum##_name(_expected), \
                   _enum##_name(_actual)); \
            return false; \
        } \
} while (0)

/**
 * \brief Assert that two strings are equal.
 *
 * If the assertion fails, an error message will be printed containing the file
 * name and line number on which the macro invocation occurs and the expected
 * and actual strings, and the surrounding function will `return false;`.
 *
 * Because \a _expected, \a _actual, and \a _type will be evaluated multiple
 * times by the macro, do not pass expressions having side effects.
 *
 * \param [in] _expected the expected string
 * \param [in] _actual the actual string
 */
#define TEST_ASSERT_STR_EQ(_expected, _actual) do { \
        if (strcmp((_expected), (_actual)) != 0) \
        { \
            int32_t _expectedLen = strlen(_expected); \
            int32_t _actualLen = strlen(_actual); \
            int32_t _expectedEscapedLen = _expectedLen * 4 + 1; \
            int32_t _actualEscapedLen = _actualLen * 4 + 1; \
            char *_expectedEscaped = (char *) malloc(_expectedEscapedLen); \
            char *_actualEscaped = (char *) malloc(_actualEscapedLen); \
            rbr_strnesccntrl(_expectedEscaped, \
                             _expected, \
                             _expectedEscapedLen); \
            rbr_strnesccntrl(_actualEscaped, \
                             _actual, \
                             _actualEscapedLen); \
            \
            printf(" assertion failure at %s:%d:\n" \
                   "\texpected \"%s\"\n" \
                   "\t  actual \"%s\"", \
                   __FILE__, \
                   __LINE__, \
                   _expectedEscaped, \
                   _actualEscaped); \
            \
            free(_expectedEscaped); \
            free(_actualEscaped); \
            return false; \
        } \
} while (0)

/** \brief The size of the write buffer used for tests. */
#define TESTIOBUFFERS_WRITE_BUFFER_SIZE 4096

/** \brief The characters terminating an instrument command. */
#define COMMAND_TERMINATOR "\r"

/** \brief The characters terminating an instrument command response. */
#define RESPONSE_TERMINATOR "\r\n"

/** \brief The maximum number of parsed samples to buffer. */
#define TESTPARSERBUFFERS_SAMPLES_MAX 64

/** \brief The maximum number of parsed events to buffer. */
#define TESTPARSERBUFFERS_EVENTS_MAX 64

/** \brief The size of the array for expectedCommand and response used in tests*/
#define COMMAND_RESPONSE_SIZE 1024

/**
 * \brief Declare an empty RBRInstrumentGen4LabelList named \a name over a
 * buffer of \a size_ entries declared alongside it as `name##Buffer`.
 */
#define RBRINSTRUMENTGEN4_LABEL_LIST_DECL(name, size_) \
    RBRInstrumentGen4Label name##Buffer[size_]; \
    RBRInstrumentGen4LabelList name = (RBRInstrumentGen4LabelList) { \
        .size = (size_), \
        .count = 0, \
        .labels = name##Buffer \
    }

/**
 * \brief Declare an empty RBRInstrumentGen4ChannelPool named \a name over a
 * buffer of \a size_ entries declared alongside it as `name##Buffer`.
 */
#define RBRINSTRUMENTGEN4_CHANNEL_POOL_DECL(name, size_) \
    RBRInstrumentGen4Channel name##Buffer[size_]; \
    RBRInstrumentGen4ChannelPool name = (RBRInstrumentGen4ChannelPool) { \
        .size = (size_), \
        .count = 0, \
        .pool = name##Buffer \
    }

/**
 * \brief Declare an empty RBRInstrumentGen4GroupPool named \a name over a
 * buffer of \a size_ entries declared alongside it as `name##Buffer`.
 */
#define RBRINSTRUMENTGEN4_GROUP_POOL_DECL(name, size_) \
    RBRInstrumentGen4Group name##Buffer[size_]; \
    RBRInstrumentGen4GroupPool name = (RBRInstrumentGen4GroupPool) { \
        .size = (size_), \
        .count = 0, \
        .pool = name##Buffer \
    }

/**
 * \brief Declare an empty RBRInstrumentGen4SchedulePool named \a name over a
 * buffer of \a size_ entries declared alongside it as `name##Buffer`.
 */
#define RBRINSTRUMENTGEN4_SCHEDULE_POOL_DECL(name, size_) \
    RBRInstrumentGen4Schedule name##Buffer[size_]; \
    RBRInstrumentGen4SchedulePool name = (RBRInstrumentGen4SchedulePool) { \
        .size = (size_), \
        .count = 0, \
        .pool = name##Buffer \
    }

/**
 * \brief Declare an empty RBRInstrumentGen4ConfigPool named \a name over a
 * buffer of \a size_ entries declared alongside it as `name##Buffer`.
 */
#define RBRINSTRUMENTGEN4_CONFIG_POOL_DECL(name, size_) \
    RBRInstrumentGen4Config name##Buffer[size_]; \
    RBRInstrumentGen4ConfigPool name = (RBRInstrumentGen4ConfigPool) { \
        .size = (size_), \
        .count = 0, \
        .pool = name##Buffer \
    }

/**
 * \brief Declare an empty RBRInstrumentGen4DatasetPool named \a name over a
 * buffer of \a size_ entries declared alongside it as `name##Buffer`.
 */
#define RBRINSTRUMENTGEN4_DATASET_POOL_DECL(name, size_) \
    RBRInstrumentGen4Dataset name##Buffer[size_]; \
    RBRInstrumentGen4DatasetPool name = (RBRInstrumentGen4DatasetPool) { \
        .size = (size_), \
        .count = 0, \
        .pool = name##Buffer \
    }

/**
 * \brief Escape control characters in a string.
 *
 * Control characters (as defined by iscntrl(3)) are replaced in the
 * destination with the sequence “<XX>”. For the carriage return (`\r`) and
 * line feed (`\n`) characters, “XX” is “CR” and “LF”, respectively. For all
 * other control characters, it is the upper-case hexadecimal representation of
 * the character.
 *
 * No more than \a num characters will be written into the \a destination,
 * inclusive of the null terminator.
 *
 * \param [out] destination the destination string buffer
 * \param [in] source the source string
 * \param [num] the maximum number of characters to write into the destination
 * \return the destination string buffer
 */
char *rbr_strnesccntrl(char *destination, const char *source, size_t num);

/**
 * \brief Prepare expectedCommand and response with correct terminator for TestIOBuffers.
 *
 * This function is usually used in tests to set parameters. 
 * 
 * Terminators are defined in tests.h as COMMAND_TERMINATOR and RESPONSE_TERMINATOR.
 * expectedCommand and response are defined in each test, and size is COMMAND_RESPONSE_SIZE.
 * expectedCommand will terminate with COMMAND_TERMINATOR, and response will terminate with RESPONSE_TERMINATOR.
 *
 * \param [in] text the string used in command and appear in response
 * \param [in] expectedCommand the command expected to be written to TESTIOBuffers->writeBuffer
 * \param [in] response the response expected to be in TESTIOBuffers->readBuffer
 */
void rbr_prepareCommandResponse(const char *text, char *expectedCommand, char *response);

/**
 * \brief The I/O buffers used for tests.
 */
typedef struct TestIOBuffers
{
    /** \brief The instrument under test will read from this buffer. */
    const char *readBuffer;
    /** \brief The size of the test read buffer. */
    int32_t readBufferSize;
    /** \brief How far into the read buffer the instrument has read. */
    int32_t readBufferPos;
    /** \brief The instrument under test will write back into this buffer. */
    char writeBuffer[TESTIOBUFFERS_WRITE_BUFFER_SIZE];
    /** \brief How far into the write buffer the instrument has written. */
    int32_t writeBufferPos;
    /** \brief The last sample received from the test instrument. */
    RBRInstrumentGen4Sample streamSample;
} TestIOBuffers;

/**
 * \brief (Re)initialize a set of test buffers.
 *
 * If \a readBufferSize is given as 0, its length will be determined
 * automatically with `strlen()`.
 *
 * \param [in,out] buffers the buffers to initialize
 * \param [in] readBuffer the new contents of the read buffer
 * \param [in] readBufferSize the length of the read buffer
 */
void TestIOBuffers_init(TestIOBuffers *buffers,
                        const char *readBuffer,
                        int32_t readBufferSize);

/**
 * \brief Get a string name for a boolean value.
 *
 * Enables the use of TEST_ASSERT_ENUM_EQ with boolean values.
 *
 * \param [in] value the boolean value
 * \return a string name for the value
 */
const char *bool_name(bool value);

/**
 * \brief Declare a test function.
 *
 * \param [in] fn the name of the test function
 */
/* Uncrustify thinks that asterisks in macros are multiplication operators and
 * incorrectly adds spacing, so we'll turn *INDENT-OFF* just for this. */
#define _TEST(fn) bool test_##fn(RBRInstrumentGen4 *instrument, \
                                 TestIOBuffers *buffers)
/* *INDENT-ON* */


/**
 * \brief Declare a test function for Logger4-generation instruments.
 *
 * \param [in] fn the name of the test function
 */
#define TEST_LOGGER4(fn) _TEST(fn##_l4)

/**
 * \brief An instrument test to be run.
 *
 * \param instrument the instrument connection
 * \param buffers the test I/O buffers
 * \return whether the test passed
 */
typedef bool (InstrumentTestFunction)(RBRInstrumentGen4 *instrument,
                                      TestIOBuffers *buffers);

/**
 * \brief Declaration of an instrument test.
 *
 * Instances are generated in `tests.c` at build time.
 */
typedef struct InstrumentTest
{
    /** \brief The name of the test. */
    const char *name;
    /** \brief The instrument generation to which this test applies. */
    RBRInstrumentGen4Generation generation;
    /** \brief The test to be run. */
    InstrumentTestFunction *function;
} InstrumentTest;

/**
 * \brief All the instrument tests to run.
 *
 * Generated in `tests.c` at build time.
 */
extern InstrumentTest instrumentTests[];

/**
 * \brief Declare a test parser configuration.
 *
 * \param [in] cfg the name of the configuration
 */
#define TEST_PARSER_CONFIG(cfg) const RBRParserGen4Config test_##cfg##_parser_config

/**
 * \brief Declare a parser test function.
 *
 * \param [in] fn the name of the test function
 * \param [in] cfg the name of the configuration used by the test
 */
/* *INDENT-OFF* */
#define TEST_PARSER(fn, cfg) bool test_##fn##_parser( \
    RBRParserGen4 *parser, \
    TestParserBuffers *buffers)
/* *INDENT-ON* */

/**
 * \brief The results of test parsings.
 */
typedef struct TestParserBuffers
{
    /** \brief The length of TestParserBuffers.samples. */
    int32_t samplesLength;
    /** \brief Parsed samples. */
    RBRInstrumentGen4Sample samples[TESTPARSERBUFFERS_SAMPLES_MAX];
    /** \brief The length of TestParserBuffers.events. */
    int32_t eventsLength;
    /** \brief Parsed events. */
    RBRInstrumentGen4Event events[TESTPARSERBUFFERS_EVENTS_MAX];
} TestParserBuffers;

/**
 * \brief A parser test to be run.
 *
 * \param parser the parser to test
 * \param buffers the parser result buffers
 * \return whether the test passed
 */
typedef bool (ParserTestFunction)(RBRParserGen4 *parser,
                                  TestParserBuffers *buffers);

/**
 * \brief Declaration of a parser test.
 *
 * Instances are generated in `tests.c` at build time.
 */
typedef struct ParserTest
{
    /** \brief The name of the test. */
    const char *name;
    /** \brief The parser configuration. */
    const RBRParserGen4Config *config;
    /** \brief The test to be run. */
    ParserTestFunction *function;
} ParserTest;

/**
 * \brief All the parser tests to run.
 *
 * Generated in `tests.c` at build time.
 */
extern ParserTest parserTests[];

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_TESTS_H */
