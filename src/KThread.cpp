/*
 * KThread.cpp
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <stdio.h> // TEMPORARY
#include <stdlib.h>
#include <mem.h>
#include <dos.h>

#include "Macro.h"
#include "KThread.h"
#include "Thread.h"
#include "System.h"

unsigned PCB::capacity = initialObjectCapacity, PCB::count = 0;
PCB** PCB::objects = 0;

PCB::PCB()
{
    initialize(0, 0, 0, defaultTimeSlice);
}

PCB::PCB(ThreadBody body, StackSize stackSize, Time timeSlice)
{
    initialize(0, body, stackSize, timeSlice);
}

PCB::PCB(Thread *userThread, StackSize stackSize, Time timeSlice)
{
    initialize(userThread, 0, stackSize, timeSlice);
}

PCB::~PCB()
{
    // Only delete the stack if it was created.
    if (mStack) delete [] mStack;
    objects[mID] = 0;
    // printf("ID to delete: %d\n", mID);
    // #ifndef BCC_BLOCK_IGNORE
    // if (objects)
    //     for (unsigned i = 0; i < count; ++i)
    //         printf("object[%d]: SEG = %d OFF = %d\n", i, FP_SEG(objects[i]), FP_OFF(objects[i]));
    // #endif
}

void PCB::start()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    System::threadPut(this);
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void PCB::waitToComplete()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    if (mState != ThreadState::Terminated)
    {
        // Only block the running thread if this thread is not terminated!
        System::running->mState = ThreadState::Blocked;
        System::running->mNext = mBlocked;
        mBlocked = (PCB*) System::running;
        System::dispatch();
    }
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void PCB::stop()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    System::running->mState = ThreadState::Terminated;
    // Unblock any blocked threads by setting their state to Ready,
    // and putting them into the scheduler.
    while (System::running->mBlocked)
    {
        PCB *current = System::running->mBlocked;
        // Setting the state to Running because threadPut will
        // then reset it to Ready. Otherwise it wont put it inside
        // the scheduler (by design).
        current->mState = ThreadState::Running;
        System::threadPut((PCB*) current);
        System::running->mBlocked = current->mNext;
        current->mNext = 0;
    }
    System::dispatch();
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void PCB::sleep(unsigned timeToSleep)
{
    printf("Time to sleep: %d\n", timeToSleep);
}

PCB* PCB::getAt(unsigned index)
{
    if (index < count) return objects[index];
    else return 0;
}

void PCB::initialize(Thread *userThread, ThreadBody body,
                     StackSize stackSize, Time timeSlice)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    if (userThread && body)
    {
        // No body, just initialize the private members.
        mStack = 0;
        mBP = mSP = mSS = 0;
    }
    else
    {
        if (stackSize < minStackSize) stackSize = minStackSize;
        if (stackSize > maxStackSize) stackSize = maxStackSize;
        stackSize /= sizeof(unsigned); // BYTE to WORD
        mStack = new unsigned[stackSize];
        if (body)
        {
            // Has body, but no user thread.
            mStack[stackSize - 1] = 0x200; // PSW
            #ifndef BCC_BLOCK_IGNORE
            mStack[stackSize - 2] = FP_SEG(body); // CS
            mStack[stackSize - 3] = FP_OFF(body); // PC
            mSS = FP_SEG(mStack + stackSize - 12); // BP
            mBP = mSP = FP_OFF(mStack + stackSize - 12); // BP
            #endif
        }
        else
        {
            // Has user thread - implicit body.
            mStack[stackSize - 5] = 0x200; // PSW
            #ifndef BCC_BLOCK_IGNORE
            mStack[stackSize - 1] = FP_SEG(userThread); // wrapper() param
            mStack[stackSize - 2] = FP_OFF(userThread); // wrapper() param
            mStack[stackSize - 6] = FP_SEG(Thread::wrapper); // CS
            mStack[stackSize - 7] = FP_OFF(Thread::wrapper); // PC
            mSP = mBP = FP_OFF(mStack + stackSize - 16); // BP
            mSS = FP_SEG(mStack + stackSize - 16); // BP
            #endif
        }
    }
    mTimeSlice = timeSlice;
    mState = ThreadState::New;
    mThread = userThread;
    mNext = mBlocked = 0;
    mID = count++;
    if (count > capacity) 
    {
        //printf("resizing\n");
        PCB **temp = (PCB**) calloc(capacity << 1, sizeof(PCB*));
        if (objects)
        {
            memcpy(temp, objects, capacity * sizeof(PCB*));
            free(objects);
        }
        if (temp)
        {
            objects = temp;
            capacity <<= 1;
        }
        else printf("Failed to resize thread object array!\n");
    }
    if (objects)
    {
        objects[mID] = this;
        // DEBUG ONLY!!! REMOVE!!!
        // #ifndef BCC_BLOCK_IGNORE
        // for (unsigned i = 0; i < count; ++i)
        //     printf("object[%d]: SEG = %d OFF = %d\n", i, FP_SEG(objects[i]), FP_OFF(objects[i]));
        // #endif
    }
    else printf("Invalid thread object array!\n");
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}