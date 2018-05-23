/*
 * System.cpp
 *
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

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
InterruptRoutine System::oldTimerRoutine = 0;
volatile SysCallData *System::callData = 0;
volatile void *System::callResult = 0;
volatile unsigned System::kernelMode = 0, System::forbidPreemption = 0,
                  System::timerChangeContext = 0, System::systemChangeContext = 0,
                  System::tickCount = 0, System::readyThreadCount = 0;
volatile PCB *System::idle = 0, *System::running = 0,
             *System::runningKernelThread = 0;
volatile PCB *System::prioritized = 0, *System::sleeping = 0;

// Temporary variables for timer routine.
volatile unsigned tempBP, tempSP, tempSS;
volatile PCB *temp;

void System::initialize()
{
    // Modifying timer IVT entry and initializing sysCall IVT entry.
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    oldTimerRoutine = getvect(TimerEntry);
    setvect(TimerEntry, newTimerRoutine);
    setvect(NewTimerEntry, oldTimerRoutine);
    setvect(SysCallEntry, sysCallRoutine);
    asmUnlock();
    #endif

    // Initializing object arrays.
    PCB::objects = (PCB**) calloc(PCB::capacity, sizeof(PCB*));

    // Initializing internal kernel threads.
    idle = new PCB(idleBody, 0, 1);
    idle->mState = ThreadState::Ready;
    running = new PCB();
    running->mState = ThreadState::Running;
    running->mTimeSlice = 20; // remove this as time goes on
    runningKernelThread = new PCB(kernelBody);
    runningKernelThread->mState = ThreadState::Running;

    // Initializing system variables.
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

    // Disposing of the dynamically created objects.
    delete idle; idle = 0;
    delete running; running = 0;
    delete runningKernelThread; runningKernelThread = 0;
    
    // Disposing of the object arrays.
    free(PCB::objects); PCB::objects = 0;
}

void System::threadPut(PCB *thread)
{
    // We can only put new or running threads into the scheduler.
    if (thread->mState == ThreadState::Ready) return;
    if (thread->mState == ThreadState::Blocked) return;
    if (thread->mState == ThreadState::Terminated) return;
    // Cannot put the idle thread into the scheduler!
    if (thread == idle) return;
    thread->mState = ThreadState::Ready;
    readyThreadCount++;
    Scheduler::put(thread);
    // printf("Put: ID = %d, timeSlice = %d\n", thread->mID, thread->mTimeSlice);
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
    else thread = (PCB*) idle;
    thread->mState = ThreadState::Running;
    // printf("Get: ID = %d, timeSlice = %d\n", thread->mID, thread->mTimeSlice);
    return thread;
}

void System::dispatch()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    if (System::kernelMode) System::systemChangeContext = 1;
    else
    {
        timerChangeContext = 1;
        #ifndef BCC_BLOCK_IGNORE
        asmInterrupt(TimerEntry);
        #endif
    }
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void* System::getCallResult()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    void *result = (void*) callResult;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
    return result;
}

void interrupt System::newTimerRoutine(...)
{
    if (!timerChangeContext)
    {
        tick();
        #ifndef BCC_BLOCK_IGNORE
        asmInterrupt(NewTimerEntry);
        #endif
        if (tickCount > 0) tickCount--;
        // Only set the flag if there is at least one Ready
        // thread or the running thread is Terminated (we need
        // to switch to the idle thread).
        timerChangeContext =
            tickCount == 0 && (readyThreadCount > 0 ||
            running->mState == ThreadState::Terminated);
    }
    // If a context change is required and preemption is allowed,
    // and there is at least one Ready thread or the running thread
    // is Terminated (we need to switch to the idle thread).
    if (timerChangeContext && !forbidPreemption &&
        (readyThreadCount > 0 || running->mState == ThreadState::Terminated))
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
        threadPut((PCB*) running);
        running = threadGet();

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

        // If thread timeSlice is 0, forbid preemption until
        // something else changes the context.
        forbidPreemption = running->mTimeSlice == 0;
        tickCount = running->mTimeSlice;
        timerChangeContext = 0;
    }
    if (sleeping && --sleeping->mTimeLeft == 0)
    {
        // printf("Preparing to wake up some threads!\n");
        while (sleeping && sleeping->mTimeLeft == 0)
        {
            // printf("Waking up the thread with ID = %d!\n", sleeping->mID);
            // Setting the state to Running because threadPut will
            // then reset it to Ready. Otherwise it wont put it inside
            // the scheduler (by design).
            sleeping->mState = ThreadState::Running;
            threadPut((PCB*) sleeping);
            temp = sleeping;
            sleeping = sleeping->mNext;
            temp->mNext = 0;
        }
    }
    // DEBUG INFO! REMOVE!!!
    // if (sleeping)
    // {
    //     temp = sleeping;
    //     printf("Printing the list of sleeping threads!\n");
    //     while (temp)
    //     {
    //         printf("ID = %d TimeLeft = %d\n", temp->mID, temp->mTimeLeft);
    //         temp = temp->mNext;
    //     }
    // }
}

void interrupt System::sysCallRoutine(...)
{
    if (kernelMode)
    {
        // Interrupts are now blocked, so it is safe to allow preemption.
        unlock();

        // Saving the context of the kernel thread.
        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov tempBP, bp
            mov tempSP, sp
            mov tempSS, ss
        };
        #endif
        
        runningKernelThread->mSS = tempSS;
        runningKernelThread->mSP = tempSP;
        runningKernelThread->mBP = tempBP;

        // If any of the blocking requests are made, we need to change
        // the user thread context before switching to it.
        if (systemChangeContext)
        {
            threadPut((PCB*) running);
            running = threadGet();
            // If thread timeSlice is 0, forbid preemption until
            // something else changes the context.
            forbidPreemption = running->mTimeSlice == 0;
            tickCount = running->mTimeSlice;
            systemChangeContext = 0;
        }

        // Restoring the context of the user thread.
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
        kernelMode = 0;
    }
    else
    {
        // Getting the call params.
        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov tempSS, cx
            mov tempSP, dx
        };
        callData = (SysCallData*) MK_FP(tempSS, tempSP);
        #endif
    
        // Saving the context of the user thread.
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

        // Restoring the context of the kernel thread.
        tempSS = runningKernelThread->mSS;
        tempSP = runningKernelThread->mSP;
        tempBP = runningKernelThread->mBP;

        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov ss, tempSS
            mov sp, tempSP
            mov bp, tempBP
        };
        #endif

        tickCount = runningKernelThread->mTimeSlice;
        kernelMode = 1;
    }
}

void System::idleBody()
{
    // Use unconditional jump - while (1) to avoid the need to
    // reset the thread context for the next run.
    while (1) while (!readyThreadCount);
}

void System::kernelBody()
{
    // Use unconditional jump - while (1) to avoid the need to
    // reset the kernelThread context for the next call.
    while (1)
    {
        // Interrupts are allowed, but preemption must be blocked!
        lock();
        // printf("Running kernel thread!\n");
        switch (callData->reqType)
        {
        // Thread specific requests
        case RequestType::TCreate:
        {
            // printf("Creating a new thread!\n");
            PCB *thread = new PCB((Thread*) callData->object, callData->size, callData->time);
            callResult = (volatile void*) thread->mID;
            break;
        }
        case RequestType::TDestroy:
        {
            // printf("Destroying the thread!\n");
            PCB *thread = PCB::getAt((ID) callData->object);
            if (thread == 0) printf("Invalid thread ID!\n");
            else delete thread;
            break;
        }
        case RequestType::TStart:
        {
            // printf("Starting the thread!\n");
            PCB *thread = PCB::getAt((ID) callData->object);
            if (thread == 0) printf("Invalid thread ID!\n");
            else thread->start();
            break;
        }
        case RequestType::TStop:
        {
            // printf("Stopping the running thread!\n");
            PCB::stop();
            break;
        }
        case RequestType::TWaitToComplete:
        {
            // printf("Blocking the running thread on another thread!\n");
            PCB *thread = PCB::getAt((ID) callData->object);
            if (thread == 0) printf("Invalid thread ID!\n");
            else thread->waitToComplete();
            break;
        }
        case RequestType::TSleep:
        {
            // printf("Putting the running thread to sleep!\n");
            PCB::sleep(callData->time);
            break;
        }
        case RequestType::TDispatch:
        {
            // printf("Dispatching!\n");
            systemChangeContext = 1;
            break;
        }
        // Semaphore specific requests
        case RequestType::SCreate:
        {
            // printf("Creating a new semaphore!\n");
            break;
        }
        case RequestType::SDestroy:
        {
            // printf("Destroying the semaphore!\n");
            break;
        }
        case RequestType::SWait:
        {
            // printf("Waiting on the semaphore!\n");
            break;
        }
        case RequestType::SSignal:
        {
            // printf("Signaling the semaphore!\n");
            break;
        }
        // Event specific requests
        case RequestType::ECreate:
        {
            // printf("Creating a new semaphore!\n");
            break;
        }
        case RequestType::EDestroy:
        {
            // printf("Destroying the semaphore!\n");
            break;
        }
        case RequestType::EWait:
        {
            // printf("Waiting on the semaphore!\n");
            break;
        }
        case RequestType::ESignal:
        {
            // printf("Signaling the semaphore!\n");
            break;
        }
        default: printf("Invalid system call request type!\n");
        }
        #ifndef BCC_BLOCK_IGNORE
        asmInterrupt(SysCallEntry);
        #endif
    }
}

void System::lock()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    forbidPreemption = 1;
    asmUnlock();
    #endif
}

void System::unlock()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    forbidPreemption = 0;
    asmUnlock();
    #endif
}