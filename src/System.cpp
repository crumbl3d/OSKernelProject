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
pInterrupt System::oldTimerRoutine = 0;
volatile SysCallData *System::callData = 0;
volatile unsigned System::locked = 0, System::changeContext = 0, System::restoreUserThread = 0;
volatile unsigned System::tickCount = 0, System::readyThreadCount = 0;
volatile PCB *System::initial = new PCB(), *System::idle = new PCB(idleBody, 0, 1);
volatile PCB *System::running = System::initial, *System::runningKernelThread = new PCB(kernelBody);
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
    oldTimerRoutine = getvect(TimerEntry);
    setvect(TimerEntry, newTimerRoutine);
    setvect(NewTimerEntry, oldTimerRoutine);
    setvect(SysCallEntry, sysCallRoutine);
    asmUnlock();
    #endif

    ((PCB*) initial)->setTimeSlice(20);
    ((PCB*) runningKernelThread)->setTimeSlice(0);
    tickCount = running->mTimeSlice;
}

void System::finalize()
{
    // Restoring timer IVT entry.
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    setvect(TimerEntry, oldTimerRoutine);
    asmUnlock();
    #endif
    // todo dispose all the stuff...
    delete initial;
    delete idle;
    delete runningKernelThread;
}

void System::threadStop()
{
    //printf("Exiting thread!\n");
    if (!running) return; // Exception, no running thread!
    running->mState = PCB::Terminated;
    PCB::dispatch();
}

void System::threadPut(PCB *thread)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    // Cannot put the idle thread into the Scheduler!
    if (thread != idle)
    {
        //printf("Put: SS = %d, SP = %d, BP = %d, timeSlice = %d\n", thread->mSS, thread->mSP, thread->mBP, thread->mTimeSlice);
        readyThreadCount++;
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
    if (thread) readyThreadCount--;
    else
    {
        thread = (PCB*) idle;
        printf("Idle thread!\n");
    }
    //printf("Get: SS = %d, SP = %d, BP = %d, timeSlice = %d\n", thread->mSS, thread->mSP, thread->mBP, thread->mTimeSlice);
    return thread;
}

void interrupt System::newTimerRoutine(...)
{
    if (!changeContext)
    {
        tick();
        #ifndef BCC_BLOCK_IGNORE
        asmInterrupt(NewTimerEntry);
        #endif
        if (tickCount > 0)
        {
            tickCount--;
            if (tickCount == 0) changeContext = 1;
        }
    }
    //else printf("Explicit context change required!\n");
    // If a context change is required and preemption is allowed,
    // and there is at least one Ready thread or the running thread
    // is Terminated (we need to switch to the idle thread).
    if (changeContext && !locked && (readyThreadCount > 0 || 
        running->mState == PCB::Terminated))
    {
        // Saving the context of the running thread.
        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov tempBP, bp
            mov tempSP, sp
            mov tempSS, ss
        };
        #endif

        running->mSS = tempSS;
        running->mSP = tempSP;
        running->mBP = tempBP;

        // Getting the next thread.
        if (running->mState != PCB::Terminated) threadPut((PCB*) running);
        running = threadGet();
        
        // This should not ever happen!
        if (!running) printf("ERROR: running is null!\n");

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
        changeContext = 0;
    }
}

void interrupt System::sysCallRoutine(...)
{
    //printf("System call!!!\n");
    if (restoreUserThread)
    {
        unlock();
        //printf("Restoring user thread!\n");
        // We are not saving the context of the kernel thread
        // because its only purpose is to call system functions.
        // But we must reset it completely. Therefore we do it here.
        // TODO: Use while true in kernelthread or semaphore
        //       take a look at the labs (4/5/2018 - part 2)
        ((PCB*) runningKernelThread)->reset();

        //printf("Kernel thread: SS = %d, SP = %d, BP = %d, timeSlice = %d\n", runningKernelThread->mSS, runningKernelThread->mSP, runningKernelThread->mBP, runningKernelThread->mTimeSlice);

        // AKO JE BILA ZAHTEVANA PROMENA KONTEKSTA (DODAJ
        // JOS JEDNU GLOBALNU PROMENLJIVU - NE contextChange)
        // PRVO PROMENITI RUNNING I STAVITI U SCHEDULER
        // AKO TREBA, PA TEK ONDA VRACATI KONTEKST

        // Restoring the context of the user thread.
        tempSS = running->mSS;
        tempSP = running->mSP;
        tempBP = running->mBP;

        //printf("User thread: SS = %d, SP = %d, BP = %d, timeSlice = %d\n", tempSS, tempSP, tempBP, running->mTimeSlice);

        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov ss, tempSS
            mov sp, tempSP
            mov bp, tempBP
        };
        #endif
        restoreUserThread = 0;
    }
    else
    {
        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov tempSS, cx
            mov tempSP, dx
        };
        //printf("seg: %d off %d\n", tempSS, tempSP);
        callData = (SysCallData*) MK_FP(tempSS, tempSP);
        #endif

        //printf("Restoring kernel thread!\n");
    
        // Saving the context of the current thread.
        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov tempBP, bp
            mov tempSP, sp
            mov tempSS, ss
        };
        #endif
        
        running->mSS = tempSS;
        running->mSP = tempSP;
        running->mBP = tempBP;

        //printf("User thread: SS = %d, SP = %d, BP = %d, timeSlice = %d\n", tempSS, tempSP, tempBP, running->mTimeSlice);

        // Restoring the context of the kernel thread.
        tempSS = runningKernelThread->mSS;
        tempSP = runningKernelThread->mSP;
        tempBP = runningKernelThread->mBP;

        //printf("Kernel thread: SS = %d, SP = %d, BP = %d, timeSlice = %d\n", tempSS, tempSP, tempBP, runningKernelThread->mTimeSlice);

        //if (kernelBody == runningKernelThread->mBody) printf("Good body!\n");
        
        //tempSS = FP_SEG(kernelBody);
        //tempSP = FP_OFF(kernelBody);

        //printf("kernelBody: SEG = %d OFF = %d\n", tempSS, tempSP);

        //for (tempBP = 1; tempBP <= 20; ++tempBP)
        //    printf("stack[%d] = %d\n", runningKernelThread->mStackSize - tempBP, runningKernelThread->mStack[runningKernelThread->mStackSize - tempBP]);

        //printf("Checkpoint!\n");

        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov ss, tempSS
            mov sp, tempSP
            mov bp, tempBP
        };
        #endif

        tickCount = runningKernelThread->mTimeSlice;
    }
}

void System::idleBody()
{
    // Using readyThreadCount to prevent the host OS from killing the process.
    while (!readyThreadCount);
}

void System::kernelBody()
{
    lock();
    //printf("Running kernel thread!\n");
    switch (callData->objType)
    {
        case ObjectType::Thread:
        {
            //printf("Thread call!\n");
            switch (callData->reqType)
            {
                case ThreadRequestType::Dispatch:
                {
                    //printf("Dispatching...\n");
                    // Set the flag to change the context after
                    // switching back to the user thread.
                    changeContext = 1;
                    break;
                }
                //default: printf("Default Thread system call!");
            }
            break;
        }
        case ObjectType::Semaphore:
        {
            //printf("Semaphore call!\n");
            break;
        }
        case ObjectType::Event:
        {
            //printf("Event call!\n");
            break;
        }
    }
    restoreUserThread = 1;
    #ifndef BCC_BLOCK_IGNORE
    asmInterrupt(SysCallEntry);
    #endif
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