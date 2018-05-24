/*
 * Thread.cpp
 *
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>

#include "Macro.h"
#include "Thread.h"
#include "System.h"

void Thread::start()
{
    SysCallData data;
    data.reqType = RequestType::TStart;
    data.object = (void*) mID;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

void Thread::waitToComplete()
{
    SysCallData data;
    data.reqType = RequestType::TWaitToComplete;
    data.object = (void*) mID;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

Thread::~Thread()
{
    waitToComplete();
    SysCallData data;
    data.reqType = RequestType::TDestroy;
    data.object = (void*) mID;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

void Thread::sleep(Time timeToSleep)
{
    if (timeToSleep > 0)
    {
        SysCallData data;
        data.reqType = RequestType::TSleep;
        data.time = timeToSleep;
        #ifndef BCC_BLOCK_IGNORE
        asmLock();
        sysCall(data);
        asmUnlock();
        #endif
    }
}

Thread::Thread(StackSize stackSize, Time timeSlice)
{
    SysCallData data;
    data.reqType = RequestType::TCreate;
    data.object = this;
    data.size = stackSize;
    data.time = timeSlice;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    mID = (ID) System::getCallResult();
    asmUnlock();
    #endif
}

void Thread::wrapper(Thread *running)
{
    running->run();
    SysCallData data;
    data.reqType = RequestType::TStop;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

void dispatch()
{
    SysCallData data;
    data.reqType = RequestType::TDispatch;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}