/*
 * Semaphor.cpp
 * 
 * Created on: May 24, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>

#include "Macro.h"
#include "Semaphor.h"
#include "System.h"

Semaphore::Semaphore (int init)
{
    SysCallData data;
    data.reqType = RequestType::SCreate;
    data.object = this;
    data.number = init;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    mID = (ID) System::getCallResult();
    asmUnlock();
    #endif
}

Semaphore::~Semaphore ()
{
    SysCallData data;
    data.reqType = RequestType::SDestroy;
    data.object = (void*) mID;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

int Semaphore::wait (int toBlock)
{
    SysCallData data;
    data.reqType = RequestType::SWait;
    data.object = (void*) mID;
    data.number = toBlock;
    int result = 0;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    result = (int) System::getCallResult();
    asmUnlock();
    #endif
    return result;
}

void Semaphore::signal ()
{
    SysCallData data;
    data.reqType = RequestType::SSignal;
    data.object = (void*) mID;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

int Semaphore::val () const
{
    int value;
    SysCallData data;
    data.reqType = RequestType::SValue;
    data.object = (void*) mID;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    value = (int) System::getCallResult();
    asmUnlock();
    #endif
    return value;
}