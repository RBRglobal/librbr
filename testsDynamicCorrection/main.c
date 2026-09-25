/**
 * \file main.c
 *
 * \brief Runner for the dynamic correction tests.
 *
 * \copyright
 * Copyright (c) 2026 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include <inttypes.h>
#include <stdlib.h>

#include "tests.h"

int main(void)
{
    printf("Running tests...\n");
    int success = EXIT_SUCCESS;
    int32_t testsTotal = 0;
    int32_t testsPassed = 0;
    for (int32_t i = 0; dynamicCorrectionTests[i].function != NULL; i++) {
        printf("Running dynamic correction test \"%s\"...", dynamicCorrectionTests[i].name);
        ++testsTotal;
        if (dynamicCorrectionTests[i].function()) {
            printf(" \033[32mpass\033[0m\n");
            ++testsPassed;
        } else {
            printf(" \033[31mfail\033[0m\n");
            success = EXIT_FAILURE;
        }
    }

    printf("Tests completed (%" PRIi32 "/%" PRIi32 " passed).\n", testsPassed, testsTotal);

    return success;
}
