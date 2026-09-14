Contributing
============

We are open to pull requests.
Before making one,
please ensure that your code:

- is tested_ and doesn't break any existing tests
- produces no errors or warnings
- is documented with Doxygen commands
- meets our style requirements (see below)

Style
-----

Code
~~~~

General rules:

- Favour safety in syntax.
  Avoid the sorts of syntactical shortcuts
  which can lead to errors like ```goto fail;`` <https://www.imperialviolet.org/2014/02/22/applebug.html>`__.
- Variable name lengths should scale by scope.
  Single-character or short names for short scopes;
  one- or two-word names for scopes
  which fit on the screen at once;
  and longer for module- and global-scope symbols.
  If you can see the entire context
  then it doesn't need to go in the name;
  conversely, if you can't see
  the entire context for the variable,
  then its name should help infer it.
- Fear not of line breaks.
  Insert line breaks before and after braces,
  between struct and enum members,
  and between function arguments
  where necessary.
- Code contributes to documentation.
  Annotate code with Doxygen commands
  and prefer to document any function subtleties
  which might be relevant to the user
  in the function header instead of
  in the body.

Technical pedantry:

- Use UTF-8 encoding for all files.
- Wrap comment and code lines at 79 characters.

  - Where possible to do so
    and still fit on the line,
    align when wrapping.
  - If line breaks must occur between function arguments,
    prefer to break between *all* arguments,
    not just where strictly necessary
    to adhere to the line length limit.
  - Prefer to place operators
    at the beginning of new lines,
    not trailing the previous line.

- Use Unix-style line endings (``\n``).
- Include a trailing newline
  at the end of all files.
- Use 4 spaces for indentation,
  and additional spaces for alignment.
- Use C99 syntax,
  but C89/ANSI C caution.
  This means, among other things:

  - No GNU extensions.
  - Use only compile-time constant-size arrays.
  - Use C-style (``/* ... */``) comments,
    not C++-style comments (``// ...``).

- Use types to help error checking.
  Even if your compiler doesn't mind you
  using a ``uint8_t`` interchangeably with ``RBRGen3ChannelIndex``,
  using a specific type can assist static analysis tools
  and makes type interchange mistakes easier
  for human readers to spot.
- Speaking of number types:

  - Always use specifically-sized types from ``inttypes.h``
    (e.g., ``uint8_t``, ``int32_t``)
    instead of platform-specific types
    (e.g., ``unsigned char``, ``int``)
    when dealing with data.
  - Prefer specifically-sized types elsewhere, too.
  - Prefer signed numbers over unsigned
    wherever there's any chance that the value might be used
    in arithmetic expressions
    (i.e., where the number is a number,
    not an identifier).
  - Conventionally, ``int32_t`` is used as a counter
    even where it's far larger than necessary.
    This sort of consistency helps reduce mental overhead.

- Most functions should return an ``RBRGen3Error``
  and pass actual values back to the caller via out pointers.
  Only the most trivial or pure functions
  should return a value directly.
- Use UpperCamelCase type names
  and camelCase variable names.
  Use TypeName_functionName for the names
  of method-like functions,
  and camelCase the names
  of freestanding functions.
- Keep ``*`` and ``&`` type modifiers and operators
  immediately adjacent to the affected variable,
  not separated by whitespace.
- Use spaces around operators
  and after control statements (``if``/``for``/``while``),
  but not inside parentheses.
- Everything gets braces.
  No one-line ``if``, ``for``, ``while``, etc. statements:
  put the body in braces
  so that it's obvious what it is
  and where it starts and ends.
- Braces get newlines
  (except for ``do...while``).
- Generally, everything that can have a Doxygen comment
  should have a Doxygen comment
  (typedefs, macros, structs, enums, functions, etc.).

  - In cases where the declaration and definition are separate
    (e.g., a function declaration in a header
    versus its implementation),
    only the declaration need be commented.
  - Use C/C++-style Doxygen commands (``\command``),
    not Javadoc-style commands (``@command``).
  - Files must use the ``\file`` command
    (or else no documentation gets generated!).
  - Everything except enum members
    must have at least a ``\brief`` comment.

    - Enum members can just have a comment:
      brief comments are handled no differently.

  - Functions must have at least appropriate ``\param``
    and ``\return`` comments.

A short example:

.. code-block:: c

   /**
    * \brief A short description of the function.
    *
    * A longer description of the function.
    *
    * \param [in] conn the instrument connection
    * \param [in] theseParametersAreAligned a description of the first parameter
    * \param [in] becauseTheyFitOnTheLine a description of the second parameter
    * \return a description of the return value
    */
   int32_t RBRGen3_examplePrototype(RBRGen3 *conn,
                                    int32_t theseParametersAreAligned,
                                    int32_t *becauseTheyFitOnTheLine);

   RBRGen3Error RBRGen3_exampleOfAReallyLongFunctionName(
       RBRGen3 *conn,
       int32_t theseParametersAreAllWrappedAndBroken,
       int32_t becauseTheyWouldPushPastTheLineLengthLimit,
       int32_t evenIf,
       uint8_t *someWouldFit,
       int32_t onTheSameLine)
   {
       int32_t i;

       for (i = 0; i < onTheSameLine; i++)
       {
           while (*someWouldFit)
           {
               someWouldFit++;
           }
       }
   }

If you have `Uncrustify <http://uncrustify.sourceforge.net/>`__ installed,
you can invoke the ``tools/indent.sh`` script
to make an automatic attempt
at code indentation.
The auto-indentation configuration
leans toward the conservative side;
in particular, it performs no line wrapping.

Documentation
~~~~~~~~~~~~~

Doxygen
^^^^^^^

As noted above, source code should be annotated
with Doxygen commands.
To make it easier to notice omissions,
most will result in a build error
when compiling the documentation.

Standalone Documentation Files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Source files for documentation external to the code
are edited with `Semantic Linefeeds <http://rhodesmill.org/brandon/2012/one-sentence-per-line/>`__.
We don't do this for comments in code,
as those will be consumed by end users from their editors
as well as in the Doxygen output.

.. Links below are repository-relative; the Sphinx build (docs/) redefines them.

.. _tested: docs/tests.rst
