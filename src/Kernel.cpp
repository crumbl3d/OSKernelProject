/*
 * Kernel.cpp
 *
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <stdio.h>

#include "System.h"

// extern int userMain(int argc, char* argv[]);

int userMain(int argc, char* argv[])
{
    for (long i = 0; i < 65535; ++i)
        for (long j = 0; j < 65535; ++j);
    return 0;
}

int main(int argc, char* argv[])
{
    printf("==================================================\n");
    printf("|-----------| Simple kernel project! |-----------|\n");
    printf("==================================================\n");
    printf("> Starting function userMain!\n");
    System::initialize();
    int retval = userMain(argc, argv);
    System::finalize();
    printf("> Function userMain returned: %d\n", retval);
    printf("==================================================\n");
    return retval;
}