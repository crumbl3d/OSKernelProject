/*
 * System.cpp
 *
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <stdio.h>

#include "Schedule.h"

#include "Macro.h"
#include "KThread.h"
#include "System.h"

// Only for VS Code to stop IntelliSense from being annoying.
#ifdef BCC_BLOCK_IGNORE
#define interrupt
#endif

extern void tick();

// Initializing System variables.
pInterrupt System::oldTimer = 0;
volatile SysCallData *System::callData = 0;
volatile unsigned System::locked = 0, System::changeContext = 0;
volatile unsigned System::tickCount = 0, System::threadCount = 0;
volatile PCB *System::initial = new PCB(), *System::idle = new PCB(idleBody, 0, 1);
volatile PCB *System::running = System::initial, *System::runningKernelThread = new PCB();
volatile PCB *System::prioritized = 0, *System::sleeping = 0,
             *System::blocked = 0;

// Temporary variables for context change.
volatile unsigned tempBP, tempSP, tempSS;

void System::initialize()
{
    // Modifying timer IVT entry and initializing
    // sysCall IVT entry.
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    System::oldTimer = getvect(TimerEntry);
    setvect(TimerEntry, System::newTimer);
    setvect(NewTimerEntry, System::oldTimer);
    setvect(SysCallEntry, System::sysCall);
    asmUnlock();
    #endif

    ((PCB*) initial)->setTimeSlice(2);
    tickCount = running->mTimeSlice;
}

void System::finalize()
{
    // Restoring timer IVT entry.
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    setvect(TimerEntry, System::oldTimer);
    asmUnlock();
    #endif
}

void System::sleep(unsigned timeToSleep)
{
    printf("Time to sleep: %d\n", timeToSleep);
}

void System::dispatch()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData temp;
    temp.clsName = SysCallData::Thread;
    tempSS = FP_SEG(&temp);
    tempSP = FP_OFF(&temp);
    printf("seg: %d off %d\n", tempSS, tempSP);
    asm {
        mov bx, tempSS
        mov dx, tempSP
    }
    asmInterrupt(SysCallEntry);
    asmUnlock();
    #endif
}

void System::threadExit()
{
    if (!System::running) return; // Exception, no running thread!
    System::running->mState = PCB::Terminated;
    dispatch();
}

void System::threadPut(PCB *thread)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    // Cannot put the initial nor the idle thread into the Scheduler!
    printf("Put: SS = %d, SP = %d, BP = %d, timeSlice = %d\n", thread->mSS, thread->mSP, thread->mBP, thread->mTimeSlice);
    if (thread != idle)
    {
        threadCount++;
        Scheduler::put(thread);
    }
    else printf("ERROR: idle thread\n");
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void System::threadPriorityPut(PCB *thread)
{
    // ovde stavljati u red prioritetnih niti
}

PCB* System::threadGet()
{
    // ovde obraditi prioritetne niti
    // PCB *res = Scheduler::get();
    // if (res != 0) return res;
    // return (PCB*) idle;
    PCB *thread = Scheduler::get();
    threadCount--;
    printf("Get: SS = %d, SP = %d, BP = %d, timeSlice = %d\n", thread->mSS, thread->mSP, thread->mBP, thread->mTimeSlice);
    return thread;
}

void interrupt System::newTimer(...)
{
    if (!changeContext)
    {
        tick();
        if (tickCount > 0)
        {
            tickCount--;
            if (tickCount == 0) changeContext = 1;
        }
    }
    if (changeContext && !locked)
    {
        printf(" Thread Tick\n");

        changeContext = 0;

        // Only change context if there is a different thread
        // wont work for idle fix this....
        if (threadCount > 0)
        {
            // Saving the context of the current thread.
            #ifndef BCC_BLOCK_IGNORE
            asm {
                mov tempBP, bp
                mov tempSP, sp
                mov tempSS, ss
            }
            #endif

            running->mSS = tempSS;
            running->mSP = tempSP;
            running->mBP = tempBP;

            // Getting the next thread.
            if (running->mState != PCB::Terminated) threadPut((PCB*) running);
            running = threadGet();
            
            if (!running) printf("ERROR: running is null\n");

            // Restoring the context of the next thread.
            tempSS = running->mSS;
            tempSP = running->mSP;
            tempBP = running->mBP;
            
            #ifndef BCC_BLOCK_IGNORE
            asm {
                mov ss, tempSS
                mov sp, tempSP
                mov bp, tempBP
            };
            #endif
            
            tickCount = running->mTimeSlice;
        }
    }
    #ifndef BCC_BLOCK_IGNORE
    if (!changeContext || locked) asmInterrupt(NewTimerEntry);
    #endif
}

void interrupt System::sysCall(...)
{
    lock();
    // Allow interrupts on kernel thread.
    // #ifndef BCC_BLOCK_IGNORE
    // asm sti;
    // #endif
    printf("System call!!!\n");
    // asm {
    //     mov tempSS, cx // Segment in CX
    //     mov tempSP, dx // Offset in DX
    //     pop dx
    //     pop cx
    // }
    #ifndef BCC_BLOCK_IGNORE
    // izgleda da radi za bx i dx, proveriti...
    asm {
        mov tempSS, bx
        mov tempSP, dx
    }
    printf("seg: %d off %d\n", tempSS, tempSP);
    callData = (SysCallData*) MK_FP(tempSS, tempSP);
    #endif
    if (callData == 0) printf("Invalid system call data!\n");
    switch (callData->clsName)
    {
        case SysCallData::Thread:
        {
            // todo
            // switch ((SysCallData::ThreadCallName) callData->callName)
            // {

            // }
            printf("Dispatching...\n");

            // Only change context if there is a different thread
            // wont work for idle fix this....
            if (threadCount > 0)
            {
                // Saving the context of the current thread.
                #ifndef BCC_BLOCK_IGNORE
                asm {
                    mov tempBP, bp
                    mov tempSP, sp
                    mov tempSS, ss
                }
                #endif

                running->mSS = tempSS;
                running->mSP = tempSP;
                running->mBP = tempBP;

                // Getting the next thread.
                if (running->mState != PCB::Terminated) threadPut((PCB*) running);
                running = threadGet();
                
                if (!running) printf("ERROR: running is null\n");

                // Restoring the context of the next thread.
                tempSS = running->mSS;
                tempSP = running->mSP;
                tempBP = running->mBP;

                #ifndef BCC_BLOCK_IGNORE
                asm {
                    mov ss, tempSS
                    mov sp, tempSP
                    mov bp, tempBP
                };
                #endif

                tickCount = running->mTimeSlice;
            }
            break;
        }
        default:
        {
            printf("Default task...\n");
        }
    }
    unlock();
    // prelazenje na kernel nit i dozvola prekida
    // a zabrana preuzimanja...
    
    // obrada zahteva

    // vracanje na user nit

    // obrada blokirajucih zahteva - dispatch, sleep...

    // povratak
}

void System::idleBody()
{
    // Using threadCount to prevent the host OS from killing the process.
    while (threadCount);
}

void System::lock()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    locked = 1;
    asmUnlock();
    #endif
}

void System::unlock()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    locked = 0;
    asmUnlock();
    #endif
}