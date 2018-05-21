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

ID Thread::classID = 0;

void Thread::start()
{
    mPCB->start();
}

void Thread::waitToComplete()
{
    mPCB->waitToComplete();
}

Thread::~Thread()
{
    delete mPCB;
}

void Thread::sleep(Time timeToSleep)
{
    System::sleep(timeToSleep);
}

Thread::Thread(StackSize stackSize, Time timeSlice)
{
    mPCB = new PCB(this, stackSize, timeSlice);
    mID = classID++;
}

void Thread::wrapper(Thread *running)
{
    running->run();
    // umesto ovoga mora da se zove syscall(threadExit);
    System::threadExit();
}

void dispatch()
{
    System::dispatch();
}