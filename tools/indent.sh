#! /bin/sh

## \file indent.sh
##
## \brief Source code indentation.
##
## Automatically indent source code according to the [project style guide][0].
## This script is largely a wrapper for [Uncrustify][0], and requires that said
## utility be installed.
##
## [0]: CONTRIBUTING.md
## [1]: http://uncrustify.sourceforge.net/
##
## ### Syntax
##
## ~~~{.sh}
## ./tools/indent.sh [filename ...]
## ~~~
##
## ### Usage
##
## ~~~{.sh}
## ./tools/indent.sh
## ~~~
##
## ...will reindent everything in `src/`, and
##
## ~~~{.sh}
## ./tools/indent.sh filename1 filename2
## ~~~
##
## ...will reindent just `filename1` and `filename2`.
##
## Backup files won't be made, under the assumption that Git (or some other
## form of source control) is your backup.
##
## \copyright
## Copyright (c) 2018 RBR Ltd.
## Licensed under the Apache License, Version 2.0.

SCRIPT_DIR="`dirname "$0"`"

if ! command -v uncrustify >/dev/null
then
    echo "$0: could not find uncrustify in your PATH" 1>&2
    exit 1
fi

if [ $# -ge 1 ]
then
    uncrustify --no-backup -c "$SCRIPT_DIR"/"uncrustify.cfg" "$@"
else
    uncrustify --no-backup -c "$SCRIPT_DIR"/"uncrustify.cfg" src/*.[ch] \
                                                             testsGen3/*.[ch]
fi
