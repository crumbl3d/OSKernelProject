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
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.reqType = RequestType::TStart;
    data.object = (void*) mID;
    sysCall(data);
    asmUnlock();
    #endif
}

void Thread::waitToComplete()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.reqType = RequestType::TWaitToComplete;
    data.object = (void*) mID;
    sysCall(data);
    asmUnlock();
    #endif
}

Thread::~Thread()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.reqType = RequestType::TDestroy;
    data.object = (void*) mID;
    sysCall(data);
    asmUnlock();
    #endif
}

void Thread::sleep(Time timeToSleep)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.reqType = RequestType::TSleep;
    data.time = timeToSleep;
    sysCall(data);
    asmUnlock();
    #endif
}

Thread::Thread(StackSize stackSize, Time timeSlice)
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.reqType = RequestType::TCreate;
    data.object = this;
    data.size = stackSize;
    data.time = timeSlice;
    sysCall(data);
    mID = (ID) System::getCallResult();
    asmUnlock();
    #endif
}

void Thread::wrapper(Thread *running)
{
    running->run();
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.reqType = RequestType::TStop;
    sysCall(data);
    asmUnlock();
    #endif
}

void dispatch()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    SysCallData data;
    data.reqType = RequestType::TDispatch;
    sysCall(data);
    asmUnlock();
    #endif
}