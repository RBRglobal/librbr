#! /bin/sh
# Report every library, header, and test source whose formatting differs
# from what clang-format produces from the repository's .clang-format.
status=0
for f in src/*.c src/*.h include/*.h testsGen3/*.c testsGen3/*.h \
         testsGen4/*.c testsGen4/*.h
do
    if ! clang-format --dry-run --Werror "$f" >/dev/null 2>&1
    then
        echo "$f"
        status=1
    fi
done
exit $status
