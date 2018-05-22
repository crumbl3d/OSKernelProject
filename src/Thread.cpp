/*
 * Thread.cpp
 *
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <stdio.h>

#include "Macro.h"
#include "Thread.h"
#include "KThread.h"
#include "System.h"

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
    PCB::sleep(timeToSleep);
}

Thread::Thread(StackSize stackSize, Time timeSlice)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.objType = ObjectType::Thread;
    data.reqType = ThreadRequestType::Create;
    sysCall(data);
    mID = (ID) System::getCallResult();
    asmUnlock();
    #endif
}

void Thread::wrapper(Thread *running)
{
    running->run();
    // umesto ovoga mora da se zove syscall(threadExit);
    System::threadStop();
}

void dispatch()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.objType = ObjectType::Thread;
    data.reqType = ThreadRequestType::Dispatch;
    sysCall(data);
    asmUnlock();
    #endif
}