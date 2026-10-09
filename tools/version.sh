#! /bin/sh

# Copyright (c) 2026 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

## \file version.sh
##
## \brief Determine the version number used for builds.
##
## Shell equivalent of version.cmake.
##
## Must be run from the repo root.
##
## Tries to use the output of `git describe --dirty --always`. If that fails
## (e.g., Git unavailable, or no/invalid .git metadata), uses the contents of
## the VERSION file.

# Do we have Git, and do we have repository metadata for libRBR (not for some
# parent Git repository vendoring it)?
if prefix=$(git rev-parse --show-prefix 2>/dev/null) && [ -z "$prefix" ]
then
    if version=$(git describe --dirty 2>/dev/null)
    then
        # We know about a tag, so use that (sans leading “v”, if present).
        echo "${version#v}"
        exit 0
    elif version=$(git describe --dirty --always 2>/dev/null)
    then
        # Couldn't find a tag; we're probably in a shallow clone. Prepend a
        # version number so that the output matches the same pattern.
        echo "0.0.0-0-g$version"
        exit 0
    fi
fi

if [ -r VERSION ]
then
    head -n 1 VERSION
    exit $?
fi

echo "$0: no Git information or VERSION file available!" 1>&2
exit 1
