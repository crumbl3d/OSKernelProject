/*
 * KThread.cpp
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <stdio.h> // TEMPORARY
#include <dos.h>

#include "Macro.h"
#include "KThread.h"
#include "Thread.h"
#include "System.h"

ID PCB::classID = 0;
unsigned PCB::capacity = initialObjectCapacity;
PCB** PCB::objects = new PCB*[PCB::capacity];

PCB::PCB()
{
    initialize(0, 0, 0, defaultTimeSlice);
}

PCB::PCB(pBody body, StackSize stackSize, Time timeSlice)
{
    initialize(0, body, stackSize, timeSlice);
}

PCB::PCB(Thread *thread, StackSize stackSize, Time timeSlice)
{
    initialize(thread, 0, stackSize, timeSlice);
}

PCB::~PCB()
{
    delete [] mStack;
}

void PCB::start()
{
    // If the thread is already in the
    // scheduler, do not put it again!
    if (mState == New)
    {
        mState = Ready;
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

void PCB::initialize(Thread *thread, pBody body,
                     StackSize stackSize, Time timeSlice)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    if (thread && body)
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
            mStack[stackSize - 1] = FP_SEG(thread); // wrapper() param
            mStack[stackSize - 2] = FP_OFF(thread); // wrapper() param
            mStack[stackSize - 6] = FP_SEG(Thread::wrapper); // CS
            mStack[stackSize - 7] = FP_OFF(Thread::wrapper); // PC
            mSP = mBP = FP_OFF(mStack + stackSize - 12); // BP
            mSS = FP_SEG(mStack + stackSize - 12); // BP
            #endif
        }
    }
    mTimeSlice = timeSlice;
    mState = New;
    mThread = thread;
    mNext = 0;
    mID = classID++;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}