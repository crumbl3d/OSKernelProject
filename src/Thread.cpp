/*
 * Thread.cpp
 *
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <stdio.h>

#include "Thread.h"
#include "KThread.h"
#include "System.h"

void Thread::start()
{
    mKernelThr->start();
}

void Thread::waitToComplete()
{
    mKernelThr->waitToComplete();
}

Thread::~Thread()
{
    delete mKernelThr;
}

void Thread::sleep(Time timeToSleep)
{
    System::sleep(timeToSleep);
}

Thread::Thread(StackSize stackSize, Time timeSlice)
{
    if (stackSize > maxStackSize) stackSize = maxStackSize;
    mKernelThr = new KernelThr(stackSize, timeSlice, this);
}

void dispatch()
{
    System::dispatch();
}