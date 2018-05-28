/*
 * KMain.cpp
 *
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <stdio.h>

#include "System.h"

extern int userMain (int argc, char* argv[]);

int main (int argc, char* argv[])
{
    System::initialize();
    printf("===============================================================================\n");
    printf("|--------------------------| Simple kernel project |--------------------------|\n");
    printf("===============================================================================\n");
    printf("> Starting function userMain!\n");
    int result = userMain(argc, argv);
    printf("> Function userMain returned: %d\n", result);
    printf("===============================================================================\n");
    printf("|-------------------------| Jovan Nikolov 2016/0040 |-------------------------|\n");
    printf("===============================================================================\n");
    System::finalize();
    return result;
}