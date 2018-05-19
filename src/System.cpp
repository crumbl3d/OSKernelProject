/*
 * System.cpp
 *
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <stdio.h>

#include "Macro.h"
#include "System.h"
#include "KThread.h"
#include "Thread.h"

// Only for VS Code to stop IntelliSense from crapping out.
#ifdef BCC_BLOCK_IGNORE
#define interrupt
#endif

pInterrupt System::oldTimer = 0;
volatile System::Task System::currentTask = System::None;
volatile unsigned System::counter = 20;
volatile KernelThr *System::initial = 0;
volatile KernelThr *System::idle = 0;
volatile KernelThr *System::running = 0;

void System::initialize()
{
    hijack_timer();
}

void System::finalize()
{
    restore_timer();
}

void interrupt System::newTimer(...)
{
    switch (System::currentTask)
    {
    case System::None:
        if (--System::counter == 0)
        {
            System::counter = 20;
            printf("> > > Tick!\n");
        }
        break;
    case System::Dispatch:
        printf("Dispatch!\n");
        System::currentTask = System::None;
        break;
    }
    
    #ifndef BCC_BLOCK_IGNORE
    callint(NewTimerEntry);
    #endif
}

void System::hijack_timer()
{
    #ifndef BCC_BLOCK_IGNORE
    softlock();
    System::oldTimer = getvect(TimerEntry);
    setvect(TimerEntry, System::newTimer);
    setvect(NewTimerEntry, System::oldTimer);
    softunlock();
    #endif
}

void System::restore_timer()
{
    #ifndef BCC_BLOCK_IGNORE
    softlock();
    setvect(TimerEntry, System::oldTimer);
    softunlock();
    #endif
}

void System::threadWrapper()
{
    if (!System::running) return; // Exception, no running thread!
    if (!System::running->mThread) ((KernelThr*)System::running)->run();
    else System::running->mThread->run();
    threadExit();
}

void System::threadExit()
{
    if (!System::running) return; // Exception, no running thread!
    System::running->mPCB->state = PCB::Terminated;
    dispatch();
}

void System::sleep(unsigned timeToSleep)
{
    printf("Time to sleep: %d\n", timeToSleep);
}

void System::dispatch()
{
    #ifndef BCC_BLOCK_IGNORE
    lock();
    System::currentTask = Dispatch;
    callint(TimerEntry);
    unlock();
    #endif
}