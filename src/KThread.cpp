/*
 * KThread.cpp
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <DOS.H>
#include <STDIO.H> // debug only remove

#include "Schedule.h"
#include "Macro.h"
#include "KThread.h"
#include "Thread.h"
#include "System.h"

PCB::PCB()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    mStack = 0;
    mBP = mSP = mSS = 0;
    mTimeSlice = defaultTimeSlice;
    mState = Running;
    mThread = 0;
    mNext = 0;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

PCB::PCB(Thread *thread, StackSize stackSize, Time timeSlice)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    if (stackSize < minStackSize) stackSize = minStackSize;
    if (stackSize > maxStackSize) stackSize = maxStackSize;
    stackSize /= sizeof(unsigned); // BYTE to WORD
    mStack = new unsigned[stackSize];
    mStack[stackSize - 5] = 0x200; // PSW
    #ifndef BCC_BLOCK_IGNORE
    mStack[stackSize - 1] = FP_SEG(thread); // wrapper() param
    mStack[stackSize - 2] = FP_OFF(thread); // wrapper() param
    mStack[stackSize - 6] = FP_SEG(Thread::wrapper); // CS
    mStack[stackSize - 7] = FP_OFF(Thread::wrapper); // PC
    mSP = mBP = FP_OFF(mStack + stackSize - 12); // BP
    mSS = FP_SEG(mStack + stackSize - 12); // BP
    #endif
    mTimeSlice = timeSlice;
    mState = New;
    mThread = thread;
    mNext = 0;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

PCB::PCB(pBody body, StackSize stackSize, Time timeSlice)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    if (stackSize < minStackSize) stackSize = minStackSize;
    if (stackSize > maxStackSize) stackSize = maxStackSize;
    stackSize /= sizeof(unsigned); // BYTE to WORD
    mStack = new unsigned[stackSize];
    mStack[stackSize - 1] = 0x200; // PSW
    #ifndef BCC_BLOCK_IGNORE
    mStack[stackSize - 2] = FP_SEG(body); // CS
    mStack[stackSize - 3] = FP_OFF(body); // PC
    mSS = FP_SEG(mStack + stackSize - 12);
    mBP = mSP = FP_OFF(mStack + stackSize - 12);
    #endif
    // mStack[stackSize - 12] = BP (base pointer)
    mTimeSlice = timeSlice;
    mState = New;
    mThread = 0;
    mNext = 0;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

PCB::~PCB()
{
    delete [] mStack;
}

void PCB::start()
{
    System::threadPut(this);
    //System::threadWrapper();
}

void PCB::waitToComplete()
{

}