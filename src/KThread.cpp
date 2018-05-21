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

PCB::PCB()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    mStack = 0;
    mBP = mSP = mSS = 0;
    mTimeSlice = defaultTimeSlice;
    mState = New;
    mStackSize = 0;
    mBody = 0;
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
    mStackSize = stackSize;
    mBody = 0; // This is irrelevant for this constructor.
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
    mStackSize = stackSize;
    mBody = body;
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
    mState = Ready;
    System::threadPut(this);
}

void PCB::waitToComplete()
{

}

void PCB::setTimeSlice(Time timeSlice)
{
    mTimeSlice = timeSlice;
}

void PCB::reset()
{
    mStack[mStackSize - 1] = 0x200; // PSW
    #ifndef BCC_BLOCK_IGNORE
    mStack[mStackSize - 2] = FP_SEG(mBody); // CS
    mStack[mStackSize - 3] = FP_OFF(mBody); // PC
    mSS = FP_SEG(mStack + mStackSize - 12);
    mBP = mSP = FP_OFF(mStack + mStackSize - 12);
    #endif
}

void PCB::dispatch()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.objType = ObjectType::Thread;
    data.reqType = ThreadRequestType::Dispatch;
    sysCall(data);
    asmInterrupt(TimerEntry);
    asmUnlock();
    #endif
}

void PCB::sleep(unsigned timeToSleep)
{
    printf("Time to sleep: %d\n", timeToSleep);
}