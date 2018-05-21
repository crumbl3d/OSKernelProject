/*
 * KMain.cpp
 *
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <stdio.h>

#include "Macro.h"
#include "System.h"
#include "KThread.h"
#include "Thread.h"

// extern int userMain(int argc, char* argv[]);

void runA()
{
    for (int i = 0; i < 30; ++i)
    {
        System::lock();
        printf("Thread A! i = %d\n", i);
        System::unlock();
        if (System::changeContext) dispatch();
        for (int j = 0; j < 10000; ++j)
            for (int k = 0; k < 30000; ++k);
    }
    System::threadStop();
}

void runB()
{
    for (int i = 0; i < 30; ++i)
    {
        System::lock();
        printf("Thread B! i = %d\n", i);
        System::unlock();
        if (System::changeContext) dispatch();
        for (int j = 0; j < 10000; ++j)
            for (int k = 0; k < 30000; ++k);
    }
    System::threadStop();
}

int userMain(int argc, char* argv[])
{
    PCB thrA(runA), thrB(runB);
    thrA.setTimeSlice(40);
    thrB.setTimeSlice(20);
    thrA.start();
    thrB.start();

    for (int i = 0; i < 30; ++i)
    {
        #ifndef BCC_BLOCK_IGNORE
        asmLock();
        printf("main %d\n", i);
        asmUnlock();
        #endif

        for (int j = 0; j < 30000; ++j)
            for (int k = 0; k < 30000; ++k);
    }

    return 0;
}

void tick()
{
    //printf("*");
}

int result;

int main(int argc, char* argv[])
{
    System::initialize();
    printf("===============================================================================\n");
    printf("|--------------------------| Simple kernel project |--------------------------|\n");
    printf("===============================================================================\n");
    printf("> Starting function userMain!\n");
    result = userMain(argc, argv);
    printf("> Function userMain returned: %d\n", result);
    printf("===============================================================================\n");
    printf("|-------------------------| Jovan Nikolov 2016/0040 |-------------------------|\n");
    printf("===============================================================================\n");
    System::finalize();
    return result;
}