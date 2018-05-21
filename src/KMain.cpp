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

void run()
{
    printf("Running test thread!\n");
    System::threadExit();
}

int userMain(int argc, char* argv[])
{
    PCB t(run, 4096, 20);
    t.start();

    //dispatch();

    for (long i = 0; i < 65535; ++i)
        for (long j = 0; j < 33000; ++j);

    // #ifndef BCC_BLOCK_IGNORE
    // SysCallData temp;
    // temp.clsName = SysCallData::Thread;
    // unsigned tempSS = FP_SEG(&temp);
    // unsigned tempSP = FP_OFF(&temp);
    // printf("seg: %d off %d\n", tempSS, tempSP);
    // asm {
    //     mov bx, tempSS
    //     mov dx, tempSP
    // }
    // asmInterrupt(SysCallEntry);
    // #endif
    
    return 0;
}

void tick()
{
    printf("*");
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