/*
 * System.cpp
 *
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <stdlib.h>

#include "Schedule.h"

#include "Macro.h"
#include "KThread.h"
#include "KSemap.h"
#include "KEvent.h"
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
volatile PCB *System::sleeping = 0;
PCBQueue *System::prioritized = 0;

// Temporary variables for timer routine.
static volatile unsigned tempREG = 0; //tempBP = 0, tempSP = 0, tempSS = 0, 
static volatile PCB *temp = 0;

void System::initialize()
{
    // Modifying timer IVT entry and initializing sysCall IVT entry.
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    oldTimerRoutine = getvect(TimerEntry);
    setvect(TimerEntry, newTimerRoutine);
    setvect(NewTimerEntry, oldTimerRoutine);
    setvect(SysCallEntry, sysCallRoutine);
    #endif

    // Initializing object arrays.
    PCB::objects = (PCB**) calloc(PCB::capacity, sizeof(PCB*));
    KernelSem::objects = (KernelSem**) calloc(KernelSem::capacity, sizeof(KernelSem*));

    // Initializing internal kernel threads.
    idle = new PCB(idleBody, 0, 1);
    running = new PCB();
    runningKernelThread = new PCB(kernelBody);

    idle->mState = ThreadState::Ready;
    running->mState = ThreadState::Running;
    runningKernelThread->mState = ThreadState::Running;

    // Initializing system variables.
    tickCount = running->mTimeSlice;
    
    // Initializing internal thread lists.
    prioritized = new PCBQueue();

    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void System::finalize()
{
    // Restoring timer IVT entry.
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    setvect(TimerEntry, oldTimerRoutine);
    #endif

    // Disposing of internal thread lists.
    delete prioritized;

    // Disposing of the dynamically created objects.
    delete idle; idle = 0;
    delete running; running = 0;
    delete runningKernelThread; runningKernelThread = 0;

    // Disposing of the object arrays.
    free(PCB::objects); PCB::objects = 0;
    free(KernelSem::objects); KernelSem::objects = 0;

    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void System::threadPut(PCB *thread)
{
    // We can only put new or running threads into the scheduler.
    if (thread->mState == ThreadState::Ready) return;
    if (thread->mState == ThreadState::Blocked) return;
    if (thread->mState == ThreadState::Terminated) return;
    // We cannot put the idle thread into the scheduler!
    if (thread == idle) return;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    // Change the thread state and put it into the scheduler.
    thread->mState = ThreadState::Ready;
    readyThreadCount++;
    Scheduler::put(thread);
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void System::threadPriorityPut(PCB *thread)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    // Change the thread state and put it into the scheduler.
    thread->mState = ThreadState::Ready;
    readyThreadCount++;
    prioritized->put(thread);
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

PCB* System::threadGet()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    // If there is at least one prioritized thread, return it, otherwise
    // check if there is at least one thread in the scheduler, return it,
    // otherwise return the Idle thread.
    PCB *thread = !prioritized->isEmpty() ? prioritized->get() :
       (readyThreadCount ? Scheduler::get() : (PCB*) idle);
    // Decrement the Ready thread counter if we got a thread from the
    // scheduler and set the thread state to Running.
    if (readyThreadCount) readyThreadCount--;
    thread->mState = ThreadState::Running;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
    return thread;
}

void System::dispatch()
{
    if (kernelMode) systemChangeContext = 1;
    else
    {
        timerChangeContext = 1;
        #ifndef BCC_BLOCK_IGNORE
        asmInterrupt(TimerEntry);
        #endif
    }
}

void* System::getCallResult()
{
    return (void*) callResult;
}

void interrupt System::newTimerRoutine(...)
{
    if (!timerChangeContext)
    {
        // Runs this code only if this is a normal timer tick.
        tick();
        if (tickCount > 0) --tickCount;
        if (sleeping && --sleeping->mTimeLeft == 0)
        {
            while (sleeping && sleeping->mTimeLeft == 0)
            {
                sleeping->mState = ThreadState::Running;
                threadPut((PCB*) sleeping);
                temp = sleeping;
                sleeping = sleeping->mNext;
                temp->mNext = 0;
            }
        }
		#ifndef BCC_BLOCK_IGNORE
		asmInterrupt(NewTimerEntry);
		#endif
    }
    // If an explicit context change is required or if the time slice is done
    // and either preemption is allowed and there is at least one Ready thread,
    // or the running thread is Terminated (switch to idle thread).
    if (timerChangeContext || (tickCount == 0 && running->mTimeSlice > 0 && !forbidPreemption &&
        (readyThreadCount > 0 || running->mState == ThreadState::Terminated)))
    {
        // Saving the context of the running thread.
        #ifndef BCC_BLOCK_IGNORE
        asm mov tempREG, ss;
        running->mSS = tempREG;
        asm mov tempREG, sp;
        running->mSP = tempREG;
        asm mov tempREG, bp;
        running->mBP = tempREG;
        #endif

        // Getting the next thread.
        threadPut((PCB*) running);
        running = threadGet();

        // Restoring the context of the next thread.
        #ifndef BCC_BLOCK_IGNORE
        tempREG = running->mSS;
        asm mov ss, tempREG;
        tempREG = running->mSP;
        asm mov sp, tempREG;
        tempREG = running->mBP;
        asm mov bp, tempREG;
        #endif

        tickCount = running->mTimeSlice;
        timerChangeContext = 0;
    }
}

void interrupt System::sysCallRoutine(...)
{
    if (kernelMode)
    {
        // Interrupts are now blocked, so it is safe to allow preemption.
        forbidPreemption = 0;

        // Saving the context of the kernel thread.        
        #ifndef BCC_BLOCK_IGNORE
        asm mov tempREG, ss;
        runningKernelThread->mSS = tempREG;
        asm mov tempREG, sp;
        runningKernelThread->mSP = tempREG;
        asm mov tempREG, bp;
        runningKernelThread->mBP = tempREG;
        #endif

        // If any of the blocking requests are made, we need to change
        // the user thread context before switching to it.
        if (systemChangeContext)
        {
            threadPut((PCB*) running);
            running = threadGet();
            tickCount = running->mTimeSlice;
            systemChangeContext = 0;
        }

        // Restoring the context of the user thread.
        #ifndef BCC_BLOCK_IGNORE
        tempREG = running->mSS;
        asm mov ss, tempREG;
        tempREG = running->mSP;
        asm mov sp, tempREG;
        tempREG = running->mBP;
        asm mov bp, tempREG;
        #endif

        kernelMode = 0;
    }
    else
    {
        // Getting the call params.
        #ifndef BCC_BLOCK_IGNORE
        asm {
            mov tempSEG, cx
            mov tempOFF, dx
        };
        callData = (SysCallData*) MK_FP(tempSEG, tempOFF);
        #endif

        // Saving the context of the user thread.
        #ifndef BCC_BLOCK_IGNORE
        asm mov tempREG, ss;
        running->mSS = tempREG;
        asm mov tempREG, sp;
        running->mSP = tempREG;
        asm mov tempREG, bp;
        running->mBP = tempREG;
        #endif

        // Restoring the context of the kernel thread.
        #ifndef BCC_BLOCK_IGNORE
        tempREG = runningKernelThread->mSS;
        asm mov ss, tempREG;
        tempREG = runningKernelThread->mSP;
        asm mov sp, tempREG;
        tempREG = runningKernelThread->mBP;
        asm mov bp, tempREG;
        #endif

        tickCount = runningKernelThread->mTimeSlice;
        kernelMode = 1;
    }
}

void System::idleBody()
{
    // Use unconditional jump - while (1) to avoid the need to
    // reset the thread context for the next run.
    while (1)
    {
    	while (!readyThreadCount); // Wait for a ready thread!
    	dispatch(); // Dispatch instantly, do not wait for a timer tick!
    }
}

void System::kernelBody()
{
    // Use unconditional jump - while (1) to avoid the need to
    // reset the kernelThread context for the next call.
    while (1)
    {
        // Interrupts are allowed, but preemption must be blocked!
        forbidPreemption = 1;
        switch (callData->reqType)
        {
        // Thread specific requests
        case RequestType::TCreate:
        {
            PCB *thread = new PCB((Thread*) callData->object, callData->size, callData->time);
            if (thread) callResult = (volatile void*) thread->mID;
            break;
        }
        case RequestType::TDestroy:
        {
            PCB *thread = PCB::at((ID) callData->object);
            if (thread) delete thread;
            break;
        }
        case RequestType::TStart:
        {
            PCB *thread = PCB::at((ID) callData->object);
            if (thread) thread->start();
            break;
        }
        case RequestType::TStop:
        {
            PCB::stop();
            break;
        }
        case RequestType::TWaitToComplete:
        {
            PCB *thread = PCB::at((ID) callData->object);
            if (thread) thread->waitToComplete();
            break;
        }
        case RequestType::TSleep:
        {
            PCB::sleep(callData->time);
            break;
        }
        case RequestType::TDispatch:
        {
            systemChangeContext = 1;
            break;
        }
        // Semaphore specific requests
        case RequestType::SCreate:
        {
            KernelSem *sem = new KernelSem((Semaphore*) callData->object, callData->number);
            if (sem) callResult = (volatile void*) sem->mID;
            break;
        }
        case RequestType::SDestroy:
        {
            KernelSem *sem = KernelSem::at((ID) callData->object);
            if (sem) delete sem;
            break;
        }
        case RequestType::SWait:
        {
            KernelSem *sem = KernelSem::at((ID) callData->object);
            if (sem) callResult = (volatile void*) sem->wait(callData->number);
            break;
        }
        case RequestType::SSignal:
        {
            KernelSem *sem = KernelSem::at((ID) callData->object);
            if (sem) sem->signal();
            break;
        }
        case RequestType::SValue:
        {
            KernelSem *sem = KernelSem::at((ID) callData->object);
            if (sem) callResult = (volatile void*) sem->val();
            break;
        }
        // Event specific requests
        case RequestType::ECreate:
        {
            KernelEv *event = KernelEv::at((IVTNo) callData->number);
            if (event) event->mEvent = (Event*) callData->object;
            else event = new KernelEv((Event*) callData->object, callData->number);
            break;
        }
        case RequestType::EDestroy:
        {
            KernelEv *event = KernelEv::at((IVTNo) callData->object);
            if (event) delete event;
            break;
        }
        case RequestType::EWait:
        {
            KernelEv *event = KernelEv::at((IVTNo) callData->object);
            if (event) event->wait();
            break;
        }
        case RequestType::ESignal:
        {
            KernelEv *event = KernelEv::at((IVTNo) callData->object);
            if (event) event->signal();
            break;
        }
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
    #endif
    forbidPreemption = 1;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void System::unlock()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    // Only unlock if the CPU is NOT in kernel mode!
    if (!kernelMode) forbidPreemption = 0;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}