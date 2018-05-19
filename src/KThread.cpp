/*
 * KThread.cpp
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include "KThread.h"
#include "Thread.h"
#include "System.h"

KernelThr::KernelThr(unsigned long stackSize, unsigned quantum, Thread *thread)
{
    mPCB = new PCB;
    mPCB->stack = new unsigned[stackSize / sizeof(unsigned)];
    mPCB->bp =  mPCB->sp = mPCB->ss = 0;
    mPCB->quantum = quantum;
    mPCB->state = PCB::New;
    mThread = thread;
    mNext = 0;
}

KernelThr::~KernelThr()
{
    delete mPCB;
}

void KernelThr::start()
{
    System::threadWrapper();
}

void KernelThr::waitToComplete()
{

}