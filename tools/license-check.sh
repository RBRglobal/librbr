#! /bin/sh

# Copyright (c) 2026 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

# Report every tracked file which does not carry the RBR copyright line and an
# SPDX license identifier within its first ten lines. See CONTRIBUTING.rst.
#
# Must be run from the repo root.
files=$(git ls-files) || exit 2
status=0
for f in $files
do
    case "$f" in
        LICENSE|VERSION|*.bin|*.csv) continue ;;
    esac

    if ! head -n 10 "$f" | grep -q 'Copyright (c) [0-9]\{4\} RBR Ltd\.' \
       || ! head -n 10 "$f" | grep -q 'SPDX-License-Identifier: Apache-2.0'
    then
        echo "$f"
        status=1
    fi
done
exit $status
