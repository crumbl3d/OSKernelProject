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
    initialize(this, 0, 0, 0, defaultTimeSlice);
}

PCB::PCB(pBody body, StackSize stackSize, Time timeSlice)
{
    initialize(this, 0, body, stackSize, timeSlice);
}

PCB::PCB(Thread *thread, StackSize stackSize, Time timeSlice)
{
    initialize(this, thread, 0, stackSize, timeSlice);
}

PCB::~PCB()
{
    // Only delete the stack if it was created.
    if (mStack) delete [] mStack;
    objects[mID] = 0;
}

void PCB::start()
{
    // If the thread is already in the scheduler, do not put it again!
    if (mState == ThreadState::New)
    {
        mState = ThreadState::Ready;
        System::threadPut(this);
    }
}

void PCB::waitToComplete()
{

}

void PCB::setTimeSlice(Time timeSlice)
{
    mTimeSlice = timeSlice;
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

void PCB::initialize(PCB *kernelThread, Thread *userThread, pBody body,
                     StackSize stackSize, Time timeSlice)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    if (userThread && body)
    {
        // No body, just initialize the private members.
        kernelThread->mStack = 0;
        kernelThread->mBP = kernelThread->mSP = kernelThread->mSS = 0;
    }
    else
    {
        if (stackSize < minStackSize) stackSize = minStackSize;
        if (stackSize > maxStackSize) stackSize = maxStackSize;
        stackSize /= sizeof(unsigned); // BYTE to WORD
        kernelThread->mStack = new unsigned[stackSize];
        if (body)
        {
            // Has body, but no user thread.
            kernelThread->mStack[stackSize - 1] = 0x200; // PSW
            #ifndef BCC_BLOCK_IGNORE
            kernelThread->mStack[stackSize - 2] = FP_SEG(body); // CS
            kernelThread->mStack[stackSize - 3] = FP_OFF(body); // PC
            kernelThread->mSS = FP_SEG(kernelThread->mStack + stackSize - 12); // BP
            kernelThread->mBP = kernelThread->mSP = FP_OFF(kernelThread->mStack + stackSize - 12); // BP
            #endif
        }
        else
        {
            // Has user thread - implicit body.
            kernelThread->mStack[stackSize - 5] = 0x200; // PSW
            #ifndef BCC_BLOCK_IGNORE
            kernelThread->mStack[stackSize - 1] = FP_SEG(userThread); // wrapper() param
            kernelThread->mStack[stackSize - 2] = FP_OFF(userThread); // wrapper() param
            kernelThread->mStack[stackSize - 6] = FP_SEG(Thread::wrapper); // CS
            kernelThread->mStack[stackSize - 7] = FP_OFF(Thread::wrapper); // PC
            kernelThread->mSP = kernelThread->mBP = FP_OFF(kernelThread->mStack + stackSize - 16); // BP
            kernelThread->mSS = FP_SEG(kernelThread->mStack + stackSize - 16); // BP
            #endif
        }
    }
    kernelThread->mTimeSlice = timeSlice;
    kernelThread->mState = ThreadState::New;
    kernelThread->mThread = userThread;
    kernelThread->mNext = 0;
    kernelThread->mID = count++;
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
        objects[kernelThread->mID] = kernelThread;
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